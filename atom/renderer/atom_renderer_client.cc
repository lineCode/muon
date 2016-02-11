// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/renderer/atom_renderer_client.h"

#include <string>
#include <vector>

#include "atom/browser/web_contents_preferences.h"
#include "atom/common/api/atom_bindings.h"
#include "atom/common/javascript_bindings.h"
#include "atom/common/node_bindings.h"
#include "atom/common/node_includes.h"
#include "atom/common/options_switches.h"
#include "atom/common/native_mate_converters/string16_converter.h"
#include "atom/renderer/atom_render_view_observer.h"
#include "atom/renderer/guest_view_container.h"
#include "atom/renderer/node_array_buffer_bridge.h"
#include "chrome/renderer/media/chrome_key_systems.h"
#include "chrome/renderer/pepper/pepper_helper.h"
#include "chrome/renderer/printing/print_web_view_helper.h"
#include "chrome/renderer/tts_dispatcher.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/url_constants.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_thread.h"
#include "third_party/WebKit/public/web/WebCustomElement.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebPluginParams.h"
#include "third_party/WebKit/public/web/WebKit.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "third_party/WebKit/public/web/WebRuntimeFeatures.h"
#include "third_party/WebKit/public/web/WebView.h"

#if defined(OS_WIN)
#include <shlobj.h>
#endif

namespace atom {

namespace {

// Helper class to forward the messages to the client.
class AtomRenderFrameObserver : public content::RenderFrameObserver {
 public:
  AtomRenderFrameObserver(content::RenderFrame* frame,
                          AtomRendererClient* renderer_client)
      : content::RenderFrameObserver(frame),
        renderer_client_(renderer_client) {}

  // content::RenderFrameObserver:

  void WillReleaseScriptContext(v8::Handle<v8::Context> context, int world_id) {
    renderer_client_->WillReleaseScriptContext(
        render_frame()->GetWebFrame(), context);
  }

  void DidCreateScriptContext(v8::Handle<v8::Context> context,
                              int extension_group,
                              int world_id) {
    renderer_client_->DidCreateScriptContext(
        render_frame()->GetWebFrame(), context);
  }

 private:
  AtomRendererClient* renderer_client_;

  DISALLOW_COPY_AND_ASSIGN(AtomRenderFrameObserver);
};

}  // namespace

AtomRendererClient::AtomRendererClient()
    : node_bindings_(NodeBindings::Create(false)),
      atom_bindings_(new AtomBindings),
      javascript_bindings_(new JavascriptBindings) {
}

AtomRendererClient::~AtomRendererClient() {
}

// static
void AtomRendererClient::PreSandboxStartup() {
  JavascriptBindings::PreSandboxStartup();
}

void AtomRendererClient::WebKitInitialized() {
  blink::WebCustomElement::addEmbedderCustomElementName("webview");
  blink::WebCustomElement::addEmbedderCustomElementName("browserplugin");

  if (!WebContentsPreferences::run_node())
    return;

  OverrideNodeArrayBuffer();

  node_bindings_->Initialize();
  node_bindings_->PrepareMessageLoop();

  DCHECK(!global_env);

  // Create a default empty environment which would be used when we need to
  // run V8 code out of a window context (like running a uv callback).
  v8::Isolate* isolate = blink::mainThreadIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = v8::Context::New(isolate);
  global_env = node::Environment::New(context, uv_default_loop());
}

void AtomRendererClient::RenderThreadStarted() {
  content::RenderThread::Get()->AddObserver(this);

#if defined(OS_WIN)
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  base::string16 app_id =
      command_line->GetSwitchValueNative(switches::kAppUserModelId);
  if (!app_id.empty()) {
    SetCurrentProcessExplicitAppUserModelID(app_id.c_str());
  }
#endif
}

void AtomRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  new PepperHelper(render_frame);
  new AtomRenderFrameObserver(render_frame, this);

  // Allow file scheme to handle service worker by default.
  blink::WebSecurityPolicy::registerURLSchemeAsAllowingServiceWorkers("file");
}

void AtomRendererClient::RenderViewCreated(content::RenderView* render_view) {
  new printing::PrintWebViewHelper(render_view);
  new AtomRenderViewObserver(render_view, this);
}

blink::WebSpeechSynthesizer* AtomRendererClient::OverrideSpeechSynthesizer(
    blink::WebSpeechSynthesizerClient* client) {
  return new TtsDispatcher(client);
}

bool AtomRendererClient::OverrideCreatePlugin(
    content::RenderFrame* render_frame,
    blink::WebLocalFrame* frame,
    const blink::WebPluginParams& params,
    blink::WebPlugin** plugin) {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (params.mimeType.utf8() == content::kBrowserPluginMimeType ||
      command_line->HasSwitch(switches::kEnablePlugins))
    return false;

  *plugin = nullptr;
  return true;
}

void AtomRendererClient::WillReleaseScriptContext(blink::WebFrame* frame,
                              v8::Handle<v8::Context> context) {
  if (WebContentsPreferences::run_node()) {
    node::Environment* env = node::Environment::GetCurrent(context);
    if (env == node_bindings_->uv_env()) {
      node_bindings_->set_uv_env(nullptr);
    }
  }
}

void AtomRendererClient::DidCreateScriptContext(
    blink::WebFrame* frame,
    v8::Handle<v8::Context> context) {
  GURL url(frame->document().url());

  if (url == GURL(content::kSwappedOutURL))
      return;

  if (WebContentsPreferences::run_node()) {
    // only load node in the main frame
    if (frame->parent())
      return;

    // Give the node loop a run to make sure everything is ready.
    node_bindings_->RunMessageLoop();

    // Setup node environment for each window.
    node::Environment* env = node_bindings_->CreateEnvironment(context);

    // Add atom-shell extended APIs.
    atom_bindings_->BindTo(env->isolate(), env->process_object());

    // Make uv loop being wrapped by window context.
    if (node_bindings_->uv_env() == nullptr)
      node_bindings_->set_uv_env(env);

    // Load everything.
    node_bindings_->LoadEnvironment(env);
  } else {
    v8::Isolate* isolate = context->GetIsolate();
    v8::HandleScope handle_scope(isolate);

    // Create a process object
    v8::Local<v8::Object> process = v8::Object::New(isolate);

    // attach the atom bindings
    atom_bindings_->BindTo(isolate, process);

    // attach the native function bindings
    javascript_bindings_->BindTo(isolate, process);
  }
}

bool AtomRendererClient::ShouldFork(blink::WebLocalFrame* frame,
                                    const GURL& url,
                                    const std::string& http_method,
                                    bool is_initial_navigation,
                                    bool is_server_redirect,
                                    bool* send_referrer) {
  if (WebContentsPreferences::run_node()) {
    *send_referrer = true;
    return http_method == "GET" && !is_server_redirect;
  }

  return false;
}

content::BrowserPluginDelegate* AtomRendererClient::CreateBrowserPluginDelegate(
    content::RenderFrame* render_frame,
    const std::string& mime_type,
    const GURL& original_url) {
  if (mime_type == content::kBrowserPluginMimeType) {
    return new GuestViewContainer(render_frame);
  } else {
    return nullptr;
  }
}

void AtomRendererClient::AddKeySystems(
    std::vector<media::KeySystemInfo>* key_systems) {
  AddChromeKeySystems(key_systems);
}

}  // namespace atom
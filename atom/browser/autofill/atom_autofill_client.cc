// Copyright (c) 2016 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/autofill/atom_autofill_client.h"

#include <utility>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "build/build_config.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/prefs/pref_service.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(autofill::AtomAutofillClient);

namespace autofill {

AtomAutofillClient::AtomAutofillClient(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents) {
  DCHECK(web_contents);
}

AtomAutofillClient::~AtomAutofillClient() {
}

PersonalDataManager* AtomAutofillClient::GetPersonalDataManager() {
  // content::BrowserContext* browser_context = web_contents()->GetBrowserContext());
  // return PersonalDataManagerFactory::GetForBrowserContext(
  //     browser_context->GetOriginalProfile());
  return nullptr;
}

scoped_refptr<AutofillWebDataService> AtomAutofillClient::GetDatabase() {
  scoped_refptr<AutofillWebDataService> service;
  // Profile* profile =
  //     Profile::FromBrowserContext(web_contents()->GetBrowserContext());
  // return WebDataServiceFactory::GetAutofillWebDataForProfile(
  //     profile, ServiceAccessType::EXPLICIT_ACCESS);
  return service;
}

PrefService* AtomAutofillClient::GetPrefs() {
  // return user_prefs::UserPrefs::Get(web_contents()->GetBrowserContext());
  return nullptr;
}

sync_driver::SyncService* AtomAutofillClient::GetSyncService() {
  return nullptr;
}

IdentityProvider* AtomAutofillClient::GetIdentityProvider() {
  return nullptr;
}

rappor::RapporService* AtomAutofillClient::GetRapporService() {
  return nullptr;
}

void AtomAutofillClient::ShowAutofillSettings() {
}

void AtomAutofillClient::ShowUnmaskPrompt(
    const CreditCard& card,
    UnmaskCardReason reason,
    base::WeakPtr<CardUnmaskDelegate> delegate) {
}

void AtomAutofillClient::OnUnmaskVerificationResult(
    PaymentsRpcResult result) {
}

void AtomAutofillClient::ConfirmSaveCreditCardLocally(
    const CreditCard& card,
    const base::Closure& callback) {
}

void AtomAutofillClient::ConfirmSaveCreditCardToCloud(
    const CreditCard& card,
    std::unique_ptr<base::DictionaryValue> legal_message,
    const base::Closure& callback) {
}

void AtomAutofillClient::LoadRiskData(
    const base::Callback<void(const std::string&)>& callback) {
}

bool AtomAutofillClient::HasCreditCardScanFeature() {
  return false;
}

void AtomAutofillClient::ScanCreditCard(
    const CreditCardScanCallback& callback) {
}

void AtomAutofillClient::ShowAutofillPopup(
    const gfx::RectF& element_bounds,
    base::i18n::TextDirection text_direction,
    const std::vector<autofill::Suggestion>& suggestions,
    base::WeakPtr<AutofillPopupDelegate> delegate) {
}

void AtomAutofillClient::UpdateAutofillPopupDataListValues(
    const std::vector<base::string16>& values,
    const std::vector<base::string16>& labels) {
}

void AtomAutofillClient::HideAutofillPopup() {
}

bool AtomAutofillClient::IsAutocompleteEnabled() {
  // For browser, Autocomplete is always enabled as part of Autofill.
  return GetPrefs()->GetBoolean(prefs::kAutofillEnabled);
}

// void AtomAutofillClient::MainFrameWasResized(bool width_changed) {
//   HideAutofillPopup();
// }

// void AtomAutofillClient::WebContentsDestroyed() {
//   HideAutofillPopup();
// }

// void AtomAutofillClient::OnZoomChanged(
//     const ui_zoom::ZoomController::ZoomChangedEventData& data) {
//   HideAutofillPopup();
// }

void AtomAutofillClient::PropagateAutofillPredictions(
    content::RenderFrameHost* rfh,
    const std::vector<autofill::FormStructure*>& forms) {
}

void AtomAutofillClient::DidFillOrPreviewField(
    const base::string16& autofilled_value,
    const base::string16& profile_full_name) {
}

void AtomAutofillClient::OnFirstUserGestureObserved() {
}

bool AtomAutofillClient::IsContextSecure(const GURL& form_origin) {
  return false;
}

}  // namespace autofill
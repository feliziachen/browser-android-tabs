// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/payments/autofill_wallet_model_type_controller.h"

#include <utility>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "components/autofill/core/common/autofill_prefs.h"
#include "components/prefs/pref_service.h"
#include "components/sync/driver/sync_auth_util.h"
#include "components/sync/driver/sync_service.h"
#include "google_apis/gaia/google_service_auth_error.h"

namespace browser_sync {

AutofillWalletModelTypeController::AutofillWalletModelTypeController(
    syncer::ModelType type,
    std::unique_ptr<syncer::ModelTypeControllerDelegate> delegate_on_disk,
    PrefService* pref_service,
    syncer::SyncService* sync_service)
    : ModelTypeController(type, std::move(delegate_on_disk)),
      pref_service_(pref_service),
      sync_service_(sync_service) {
  DCHECK(type == syncer::AUTOFILL_WALLET_DATA ||
         type == syncer::AUTOFILL_WALLET_METADATA);
  SubscribeToPrefChanges();
  // TODO(crbug.com/906995): remove this observing mechanism once all sync
  // datatypes are stopped by ProfileSyncService, when sync is paused.
  sync_service_->AddObserver(this);
}

AutofillWalletModelTypeController::AutofillWalletModelTypeController(
    syncer::ModelType type,
    std::unique_ptr<syncer::ModelTypeControllerDelegate> delegate_on_disk,
    std::unique_ptr<syncer::ModelTypeControllerDelegate> delegate_in_memory,
    PrefService* pref_service,
    syncer::SyncService* sync_service)
    : ModelTypeController(type,
                          std::move(delegate_on_disk),
                          std::move(delegate_in_memory)),
      pref_service_(pref_service),
      sync_service_(sync_service) {
  DCHECK(type == syncer::AUTOFILL_WALLET_DATA ||
         type == syncer::AUTOFILL_WALLET_METADATA);
  SubscribeToPrefChanges();
  // TODO(crbug.com/906995): remove this observing mechanism once all sync
  // datatypes are stopped by ProfileSyncService, when sync is paused.
  sync_service_->AddObserver(this);
}

AutofillWalletModelTypeController::~AutofillWalletModelTypeController() {
  sync_service_->RemoveObserver(this);
}

void AutofillWalletModelTypeController::Stop(
    syncer::ShutdownReason shutdown_reason,
    StopCallback callback) {
  DCHECK(CalledOnValidThread());
  switch (shutdown_reason) {
    case syncer::STOP_SYNC:
      // Special case: For AUTOFILL_WALLET_DATA and AUTOFILL_WALLET_METADATA, we
      // want to clear all data even when Sync is stopped temporarily.
      shutdown_reason = syncer::DISABLE_SYNC;
      break;
    case syncer::DISABLE_SYNC:
    case syncer::BROWSER_SHUTDOWN:
      break;
  }
  ModelTypeController::Stop(shutdown_reason, std::move(callback));
}

bool AutofillWalletModelTypeController::ReadyForStart() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetBoolean(
             autofill::prefs::kAutofillWalletImportEnabled) &&
         pref_service_->GetBoolean(
             autofill::prefs::kAutofillCreditCardEnabled) &&
         !syncer::IsWebSignout(sync_service_->GetAuthError());
}

void AutofillWalletModelTypeController::OnUserPrefChanged() {
  DCHECK(CalledOnValidThread());
  sync_service_->ReadyForStartChanged(type());
}

void AutofillWalletModelTypeController::SubscribeToPrefChanges() {
  pref_registrar_.Init(pref_service_);
  pref_registrar_.Add(
      autofill::prefs::kAutofillWalletImportEnabled,
      base::BindRepeating(&AutofillWalletModelTypeController::OnUserPrefChanged,
                          base::Unretained(this)));
  pref_registrar_.Add(
      autofill::prefs::kAutofillCreditCardEnabled,
      base::BindRepeating(&AutofillWalletModelTypeController::OnUserPrefChanged,
                          base::Unretained(this)));
}

void AutofillWalletModelTypeController::OnStateChanged(
    syncer::SyncService* sync) {
  DCHECK(CalledOnValidThread());
  sync_service_->ReadyForStartChanged(type());
}

}  // namespace browser_sync

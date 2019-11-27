/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <unordered_map>
#include <utility>

#include "hci/classic_device.h"
#include "l2cap/classic/l2cap_classic_module.h"
#include "l2cap/le/l2cap_le_module.h"
#include "os/handler.h"
#include "security/channel/security_manager_channel.h"
#include "security/pairing/classic_pairing_handler.h"
#include "security/record/security_record.h"

namespace bluetooth {
namespace security {

class ISecurityManagerListener;

namespace internal {

class SecurityManagerImpl : public channel::ISecurityManagerChannelListener {
 public:
  explicit SecurityManagerImpl(os::Handler* security_handler, l2cap::le::L2capLeModule* l2cap_le_module,
                               l2cap::classic::L2capClassicModule* l2cap_classic_module,
                               channel::SecurityManagerChannel* security_manager_channel)
      : security_handler_(security_handler), l2cap_le_module_(l2cap_le_module),
        l2cap_classic_module_(l2cap_classic_module), security_manager_channel_(security_manager_channel) {}
  virtual ~SecurityManagerImpl() = default;

  // All APIs must be invoked in SM layer handler

  /**
   * Initialize the security record map from an internal device database.
   */
  void Init();

  /**
   * Checks the device for existing bond, if not bonded, initiates pairing.
   *
   * @param device pointer to device we want to bond with
   * @return true if bonded or pairing started successfully, false if currently pairing
   */
  void CreateBond(hci::AddressWithType device);

  /* void CreateBond(std::shared_ptr<hci::LeDevice> device); */

  /**
   * Cancels the pairing process for this device.
   *
   * @param device pointer to device with which we want to cancel our bond
   * @return <code>true</code> if successfully stopped
   */
  void CancelBond(hci::AddressWithType device);

  /* void CancelBond(std::shared_ptr<hci::LeDevice> device); */

  /**
   * Disassociates the device and removes the persistent LTK
   *
   * @param device pointer to device we want to forget
   * @return true if removed
   */
  void RemoveBond(hci::AddressWithType device);

  /* void RemoveBond(std::shared_ptr<hci::LeDevice> device); */

  /**
   * Register to listen for callback events from SecurityManager
   *
   * @param listener ISecurityManagerListener instance to handle callbacks
   */
  void RegisterCallbackListener(ISecurityManagerListener* listener, os::Handler* handler);

  /**
   * Unregister listener for callback events from SecurityManager
   *
   * @param listener ISecurityManagerListener instance to unregister
   */
  void UnregisterCallbackListener(ISecurityManagerListener* listener);

  // ISecurityManagerChannel
  void OnHciEventReceived(hci::EventPacketView packet) override;

  void OnPairingHandlerComplete(hci::Address address);

 protected:
  std::vector<std::pair<ISecurityManagerListener*, os::Handler*>> listeners_;
  void NotifyDeviceBonded(hci::AddressWithType device);
  void NotifyDeviceBondFailed(hci::AddressWithType device);
  void NotifyDeviceUnbonded(hci::AddressWithType device);

 private:
  template <class T>
  void HandleEvent(T packet);

  std::shared_ptr<record::SecurityRecord> CreateSecurityRecord(hci::Address address);
  void DispatchPairingHandler(std::shared_ptr<record::SecurityRecord> record, bool locally_initiated);

  os::Handler* security_handler_ __attribute__((unused));
  l2cap::le::L2capLeModule* l2cap_le_module_ __attribute__((unused));
  l2cap::classic::L2capClassicModule* l2cap_classic_module_ __attribute__((unused));
  channel::SecurityManagerChannel* security_manager_channel_ __attribute__((unused));
  std::unordered_map<hci::Address, std::shared_ptr<record::SecurityRecord>> security_record_map_;
  std::unordered_map<hci::Address, std::shared_ptr<pairing::PairingHandler>> pairing_handler_map_;
};
}  // namespace internal
}  // namespace security
}  // namespace bluetooth

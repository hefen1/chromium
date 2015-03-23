// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/login/auth/stub_authenticator.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"

namespace chromeos {

namespace {

// As defined in /chromeos/dbus/cryptohome_client.cc.
static const char kUserIdHashSuffix[] = "-hash";

}  // anonymous namespace

StubAuthenticator::StubAuthenticator(AuthStatusConsumer* consumer,
                                     const UserContext& expected_user_context)
    : Authenticator(consumer),
      expected_user_context_(expected_user_context),
      message_loop_(base::MessageLoopProxy::current()) {
}

void StubAuthenticator::CompleteLogin(content::BrowserContext* context,
                                      const UserContext& user_context) {
  authentication_context_ = context;
  if (expected_user_context_ != user_context)
    NOTREACHED();
  OnAuthSuccess();
}

void StubAuthenticator::AuthenticateToLogin(content::BrowserContext* context,
                                            const UserContext& user_context) {
  authentication_context_ = context;
  if (user_context == expected_user_context_) {
    message_loop_->PostTask(
        FROM_HERE, base::Bind(&StubAuthenticator::OnAuthSuccess, this));
    return;
  }
  GoogleServiceAuthError error(
      GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS);
  message_loop_->PostTask(
      FROM_HERE, base::Bind(&StubAuthenticator::OnAuthFailure, this,
                            AuthFailure::FromNetworkAuthFailure(error)));
}

void StubAuthenticator::AuthenticateToUnlock(const UserContext& user_context) {
  AuthenticateToLogin(NULL /* not used */, user_context);
}

void StubAuthenticator::LoginAsSupervisedUser(const UserContext& user_context) {
  UserContext new_user_context = user_context;
  new_user_context.SetUserIDHash(user_context.GetUserID() + kUserIdHashSuffix);
  consumer_->OnAuthSuccess(new_user_context);
}

void StubAuthenticator::LoginOffTheRecord() {
  consumer_->OnOffTheRecordAuthSuccess();
}

void StubAuthenticator::LoginAsPublicSession(const UserContext& user_context) {
  UserContext logged_in_user_context = user_context;
  logged_in_user_context.SetIsUsingOAuth(false);
  logged_in_user_context.SetUserIDHash(logged_in_user_context.GetUserID() +
                                       kUserIdHashSuffix);
  consumer_->OnAuthSuccess(logged_in_user_context);
}

void StubAuthenticator::LoginAsKioskAccount(const std::string& app_user_id,
                                            bool use_guest_mount) {
  UserContext user_context(expected_user_context_.GetUserID());
  user_context.SetIsUsingOAuth(false);
  user_context.SetUserIDHash(expected_user_context_.GetUserID() +
                             kUserIdHashSuffix);
  consumer_->OnAuthSuccess(user_context);
}

void StubAuthenticator::OnAuthSuccess() {
  // If we want to be more like the real thing, we could save the user ID
  // in AuthenticateToLogin, but there's not much of a point.
  UserContext user_context(expected_user_context_);
  user_context.SetUserIDHash(expected_user_context_.GetUserID() +
                             kUserIdHashSuffix);
  consumer_->OnAuthSuccess(user_context);
}

void StubAuthenticator::OnAuthFailure(const AuthFailure& failure) {
  consumer_->OnAuthFailure(failure);
}

void StubAuthenticator::RecoverEncryptedData(const std::string& old_password) {
}

void StubAuthenticator::ResyncEncryptedData() {
}

void StubAuthenticator::SetExpectedCredentials(
    const UserContext& user_context) {
  expected_user_context_ = user_context;
}

StubAuthenticator::~StubAuthenticator() {
}

}  // namespace chromeos

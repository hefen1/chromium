// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/gcm_driver/gcm_driver_desktop.h"

#include <utility>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/files/file_path.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/metrics/histogram.h"
#include "base/sequenced_task_runner.h"
#include "base/threading/sequenced_worker_pool.h"
#include "components/gcm_driver/gcm_account_mapper.h"
#include "components/gcm_driver/gcm_app_handler.h"
#include "components/gcm_driver/gcm_channel_status_syncer.h"
#include "components/gcm_driver/gcm_client_factory.h"
#include "components/gcm_driver/gcm_delayed_task_controller.h"
#include "components/gcm_driver/system_encryptor.h"
#include "google_apis/gcm/engine/account_mapping.h"
#include "net/base/ip_endpoint.h"
#include "net/url_request/url_request_context_getter.h"

#if defined(OS_CHROMEOS)
#include "components/timers/alarm_timer.h"
#endif

namespace gcm {

class GCMDriverDesktop::IOWorker : public GCMClient::Delegate {
 public:
  // Called on UI thread.
  IOWorker(const scoped_refptr<base::SequencedTaskRunner>& ui_thread,
           const scoped_refptr<base::SequencedTaskRunner>& io_thread);
  virtual ~IOWorker();

  // Overridden from GCMClient::Delegate:
  // Called on IO thread.
  void OnRegisterFinished(const std::string& app_id,
                          const std::string& registration_id,
                          GCMClient::Result result) override;
  void OnUnregisterFinished(const std::string& app_id,
                            GCMClient::Result result) override;
  void OnSendFinished(const std::string& app_id,
                      const std::string& message_id,
                      GCMClient::Result result) override;
  void OnMessageReceived(const std::string& app_id,
                         const GCMClient::IncomingMessage& message) override;
  void OnMessagesDeleted(const std::string& app_id) override;
  void OnMessageSendError(
      const std::string& app_id,
      const GCMClient::SendErrorDetails& send_error_details) override;
  void OnSendAcknowledged(const std::string& app_id,
                          const std::string& message_id) override;
  void OnGCMReady(const std::vector<AccountMapping>& account_mappings,
                  const base::Time& last_token_fetch_time) override;
  void OnActivityRecorded() override;
  void OnConnected(const net::IPEndPoint& ip_endpoint) override;
  void OnDisconnected() override;

  // Called on IO thread.
  void Initialize(
      scoped_ptr<GCMClientFactory> gcm_client_factory,
      const GCMClient::ChromeBuildInfo& chrome_build_info,
      const base::FilePath& store_path,
      const scoped_refptr<net::URLRequestContextGetter>& request_context,
      const scoped_refptr<base::SequencedTaskRunner> blocking_task_runner);
  void Start(GCMClient::StartMode start_mode,
             const base::WeakPtr<GCMDriverDesktop>& service);
  void Stop();
  void Register(const std::string& app_id,
                const std::vector<std::string>& sender_ids);
  void Unregister(const std::string& app_id);
  void Send(const std::string& app_id,
            const std::string& receiver_id,
            const GCMClient::OutgoingMessage& message);
  void GetGCMStatistics(bool clear_logs);
  void SetGCMRecording(bool recording);

  void SetAccountTokens(
      const std::vector<GCMClient::AccountTokenInfo>& account_tokens);
  void UpdateAccountMapping(const AccountMapping& account_mapping);
  void RemoveAccountMapping(const std::string& account_id);
  void SetLastTokenFetchTime(const base::Time& time);
  void WakeFromSuspendForHeartbeat(bool wake);

  // For testing purpose. Can be called from UI thread. Use with care.
  GCMClient* gcm_client_for_testing() const { return gcm_client_.get(); }

 private:
  scoped_refptr<base::SequencedTaskRunner> ui_thread_;
  scoped_refptr<base::SequencedTaskRunner> io_thread_;

  base::WeakPtr<GCMDriverDesktop> service_;

  scoped_ptr<GCMClient> gcm_client_;

  DISALLOW_COPY_AND_ASSIGN(IOWorker);
};

GCMDriverDesktop::IOWorker::IOWorker(
    const scoped_refptr<base::SequencedTaskRunner>& ui_thread,
    const scoped_refptr<base::SequencedTaskRunner>& io_thread)
    : ui_thread_(ui_thread),
      io_thread_(io_thread) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
}

GCMDriverDesktop::IOWorker::~IOWorker() {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());
}

void GCMDriverDesktop::IOWorker::Initialize(
    scoped_ptr<GCMClientFactory> gcm_client_factory,
    const GCMClient::ChromeBuildInfo& chrome_build_info,
    const base::FilePath& store_path,
    const scoped_refptr<net::URLRequestContextGetter>& request_context,
    const scoped_refptr<base::SequencedTaskRunner> blocking_task_runner) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  gcm_client_ = gcm_client_factory->BuildInstance();

  gcm_client_->Initialize(chrome_build_info,
                          store_path,
                          blocking_task_runner,
                          request_context,
                          make_scoped_ptr<Encryptor>(new SystemEncryptor),
                          this);
}

void GCMDriverDesktop::IOWorker::OnRegisterFinished(
    const std::string& app_id,
    const std::string& registration_id,
    GCMClient::Result result) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  ui_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::RegisterFinished, service_, app_id,
                 registration_id, result));
}

void GCMDriverDesktop::IOWorker::OnUnregisterFinished(
    const std::string& app_id,
    GCMClient::Result result) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  ui_thread_->PostTask(FROM_HERE,
                       base::Bind(&GCMDriverDesktop::UnregisterFinished,
                                  service_,
                                  app_id,
                                  result));
}

void GCMDriverDesktop::IOWorker::OnSendFinished(const std::string& app_id,
                                                const std::string& message_id,
                                                GCMClient::Result result) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  ui_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::SendFinished, service_, app_id, message_id,
                 result));
}

void GCMDriverDesktop::IOWorker::OnMessageReceived(
    const std::string& app_id,
    const GCMClient::IncomingMessage& message) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  ui_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::MessageReceived,
                 service_,
                 app_id,
                 message));
}

void GCMDriverDesktop::IOWorker::OnMessagesDeleted(const std::string& app_id) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  ui_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::MessagesDeleted, service_, app_id));
}

void GCMDriverDesktop::IOWorker::OnMessageSendError(
    const std::string& app_id,
    const GCMClient::SendErrorDetails& send_error_details) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  ui_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::MessageSendError, service_, app_id,
                 send_error_details));
}

void GCMDriverDesktop::IOWorker::OnSendAcknowledged(
    const std::string& app_id,
    const std::string& message_id) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  ui_thread_->PostTask(
      FROM_HERE,
      base::Bind(
          &GCMDriverDesktop::SendAcknowledged, service_, app_id, message_id));
}

void GCMDriverDesktop::IOWorker::OnGCMReady(
    const std::vector<AccountMapping>& account_mappings,
    const base::Time& last_token_fetch_time) {
  ui_thread_->PostTask(FROM_HERE,
                       base::Bind(&GCMDriverDesktop::GCMClientReady,
                                  service_,
                                  account_mappings,
                                  last_token_fetch_time));
}

void GCMDriverDesktop::IOWorker::OnActivityRecorded() {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());
  // When an activity is recorded, get all the stats and refresh the UI of
  // gcm-internals page.
  GetGCMStatistics(false);
}

void GCMDriverDesktop::IOWorker::OnConnected(
    const net::IPEndPoint& ip_endpoint) {
  ui_thread_->PostTask(FROM_HERE,
                       base::Bind(&GCMDriverDesktop::OnConnected,
                                  service_,
                                  ip_endpoint));
}

void GCMDriverDesktop::IOWorker::OnDisconnected() {
  ui_thread_->PostTask(FROM_HERE,
                       base::Bind(&GCMDriverDesktop::OnDisconnected, service_));
}

void GCMDriverDesktop::IOWorker::Start(
    GCMClient::StartMode start_mode,
    const base::WeakPtr<GCMDriverDesktop>& service) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  service_ = service;
  gcm_client_->Start(start_mode);
}

void GCMDriverDesktop::IOWorker::Stop() {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  gcm_client_->Stop();
}

void GCMDriverDesktop::IOWorker::Register(
    const std::string& app_id,
    const std::vector<std::string>& sender_ids) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  gcm_client_->Register(app_id, sender_ids);
}

void GCMDriverDesktop::IOWorker::Unregister(const std::string& app_id) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  gcm_client_->Unregister(app_id);
}

void GCMDriverDesktop::IOWorker::Send(
    const std::string& app_id,
    const std::string& receiver_id,
    const GCMClient::OutgoingMessage& message) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  gcm_client_->Send(app_id, receiver_id, message);
}

void GCMDriverDesktop::IOWorker::GetGCMStatistics(bool clear_logs) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());
  gcm::GCMClient::GCMStatistics stats;

  if (gcm_client_.get()) {
    if (clear_logs)
      gcm_client_->ClearActivityLogs();
    stats = gcm_client_->GetStatistics();
  }

  ui_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::GetGCMStatisticsFinished, service_, stats));
}

void GCMDriverDesktop::IOWorker::SetGCMRecording(bool recording) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());
  gcm::GCMClient::GCMStatistics stats;

  if (gcm_client_.get()) {
    gcm_client_->SetRecording(recording);
    stats = gcm_client_->GetStatistics();
    stats.gcm_client_created = true;
  }

  ui_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::GetGCMStatisticsFinished, service_, stats));
}

void GCMDriverDesktop::IOWorker::SetAccountTokens(
    const std::vector<GCMClient::AccountTokenInfo>& account_tokens) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  if (gcm_client_.get())
    gcm_client_->SetAccountTokens(account_tokens);
}

void GCMDriverDesktop::IOWorker::UpdateAccountMapping(
    const AccountMapping& account_mapping) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  if (gcm_client_.get())
    gcm_client_->UpdateAccountMapping(account_mapping);
}

void GCMDriverDesktop::IOWorker::RemoveAccountMapping(
    const std::string& account_id) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  if (gcm_client_.get())
    gcm_client_->RemoveAccountMapping(account_id);
}

void GCMDriverDesktop::IOWorker::SetLastTokenFetchTime(const base::Time& time) {
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  if (gcm_client_.get())
    gcm_client_->SetLastTokenFetchTime(time);
}

void GCMDriverDesktop::IOWorker::WakeFromSuspendForHeartbeat(bool wake) {
#if defined(OS_CHROMEOS)
  DCHECK(io_thread_->RunsTasksOnCurrentThread());

  scoped_ptr<base::Timer> timer;
  if (wake)
    timer.reset(new timers::AlarmTimer(true, false));
  else
    timer.reset(new base::Timer(true, false));

  gcm_client_->UpdateHeartbeatTimer(timer.Pass());
#endif
}

GCMDriverDesktop::GCMDriverDesktop(
    scoped_ptr<GCMClientFactory> gcm_client_factory,
    const GCMClient::ChromeBuildInfo& chrome_build_info,
    const std::string& channel_status_request_url,
    const std::string& user_agent,
    PrefService* prefs,
    const base::FilePath& store_path,
    const scoped_refptr<net::URLRequestContextGetter>& request_context,
    const scoped_refptr<base::SequencedTaskRunner>& ui_thread,
    const scoped_refptr<base::SequencedTaskRunner>& io_thread,
    const scoped_refptr<base::SequencedTaskRunner>& blocking_task_runner)
    : gcm_channel_status_syncer_(
          new GCMChannelStatusSyncer(this,
                                     prefs,
                                     channel_status_request_url,
                                     user_agent,
                                     request_context)),
      signed_in_(false),
      gcm_started_(false),
      gcm_enabled_(true),
      connected_(false),
      account_mapper_(new GCMAccountMapper(this)),
      // Setting to max, to make sure it does not prompt for token reporting
      // Before reading a reasonable value from the DB, which might be never,
      // in which case the fetching will be triggered.
      last_token_fetch_time_(base::Time::Max()),
      ui_thread_(ui_thread),
      io_thread_(io_thread),
      wake_from_suspend_enabled_(false),
      weak_ptr_factory_(this) {
  gcm_enabled_ = gcm_channel_status_syncer_->gcm_enabled();

  // Create and initialize the GCMClient. Note that this does not initiate the
  // GCM check-in.
  io_worker_.reset(new IOWorker(ui_thread, io_thread));
  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::Initialize,
                 base::Unretained(io_worker_.get()),
                 base::Passed(&gcm_client_factory),
                 chrome_build_info,
                 store_path,
                 request_context,
                 blocking_task_runner));
}

GCMDriverDesktop::~GCMDriverDesktop() {
}

void GCMDriverDesktop::Shutdown() {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  Stop();
  GCMDriver::Shutdown();

  // Dispose the syncer in order to release the reference to
  // URLRequestContextGetter that needs to be done before IOThread gets
  // deleted.
  gcm_channel_status_syncer_.reset();

  io_thread_->DeleteSoon(FROM_HERE, io_worker_.release());
}

void GCMDriverDesktop::OnSignedIn() {
  signed_in_ = true;
}

void GCMDriverDesktop::OnSignedOut() {
  signed_in_ = false;
}

void GCMDriverDesktop::AddAppHandler(const std::string& app_id,
                                     GCMAppHandler* handler) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
  GCMDriver::AddAppHandler(app_id, handler);

   // Ensures that the GCM service is started when there is an interest.
  EnsureStarted(GCMClient::DELAYED_START);
}

void GCMDriverDesktop::RemoveAppHandler(const std::string& app_id) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
  GCMDriver::RemoveAppHandler(app_id);

  // Stops the GCM service when no app intends to consume it. Stop function will
  // remove the last app handler - account mapper.
  if (app_handlers().size() == 1) {
    Stop();
    gcm_channel_status_syncer_->Stop();
  }
}

void GCMDriverDesktop::AddConnectionObserver(GCMConnectionObserver* observer) {
  connection_observer_list_.AddObserver(observer);
}

void GCMDriverDesktop::RemoveConnectionObserver(
    GCMConnectionObserver* observer) {
  connection_observer_list_.RemoveObserver(observer);
}

void GCMDriverDesktop::Enable() {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  if (gcm_enabled_)
    return;
  gcm_enabled_ = true;

  EnsureStarted(GCMClient::DELAYED_START);
}

void GCMDriverDesktop::Disable() {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  if (!gcm_enabled_)
    return;
  gcm_enabled_ = false;

  Stop();
}

void GCMDriverDesktop::Stop() {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  // No need to stop GCM service if not started yet.
  if (!gcm_started_)
    return;

  account_mapper_->ShutdownHandler();
  GCMDriver::RemoveAppHandler(kGCMAccountMapperAppId);

  RemoveCachedData();

  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::Stop,
                 base::Unretained(io_worker_.get())));
}

void GCMDriverDesktop::RegisterImpl(
    const std::string& app_id,
    const std::vector<std::string>& sender_ids) {
  // Delay the register operation until GCMClient is ready.
  if (!delayed_task_controller_->CanRunTaskWithoutDelay()) {
    delayed_task_controller_->AddTask(base::Bind(&GCMDriverDesktop::DoRegister,
                                                 weak_ptr_factory_.GetWeakPtr(),
                                                 app_id,
                                                 sender_ids));
    return;
  }

  DoRegister(app_id, sender_ids);
}

void GCMDriverDesktop::DoRegister(const std::string& app_id,
                                  const std::vector<std::string>& sender_ids) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
  if (!HasRegisterCallback(app_id)) {
    // The callback could have been removed when the app is uninstalled.
    return;
  }

  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::Register,
                 base::Unretained(io_worker_.get()),
                 app_id,
                 sender_ids));
}

void GCMDriverDesktop::UnregisterImpl(const std::string& app_id) {
  // Delay the unregister operation until GCMClient is ready.
  if (!delayed_task_controller_->CanRunTaskWithoutDelay()) {
    delayed_task_controller_->AddTask(
        base::Bind(&GCMDriverDesktop::DoUnregister,
                   weak_ptr_factory_.GetWeakPtr(),
                   app_id));
    return;
  }

  DoUnregister(app_id);
}

void GCMDriverDesktop::DoUnregister(const std::string& app_id) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  // Ask the server to unregister it. There could be a small chance that the
  // unregister request fails. If this occurs, it does not bring any harm since
  // we simply reject the messages/events received from the server.
  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::Unregister,
                 base::Unretained(io_worker_.get()),
                 app_id));
}

void GCMDriverDesktop::SendImpl(const std::string& app_id,
                                const std::string& receiver_id,
                                const GCMClient::OutgoingMessage& message) {
  // Delay the send operation until all GCMClient is ready.
  if (!delayed_task_controller_->CanRunTaskWithoutDelay()) {
    delayed_task_controller_->AddTask(base::Bind(&GCMDriverDesktop::DoSend,
                                                 weak_ptr_factory_.GetWeakPtr(),
                                                 app_id,
                                                 receiver_id,
                                                 message));
    return;
  }

  DoSend(app_id, receiver_id, message);
}

void GCMDriverDesktop::DoSend(const std::string& app_id,
                              const std::string& receiver_id,
                              const GCMClient::OutgoingMessage& message) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::Send,
                 base::Unretained(io_worker_.get()),
                 app_id,
                 receiver_id,
                 message));
}

GCMClient* GCMDriverDesktop::GetGCMClientForTesting() const {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
  return io_worker_ ? io_worker_->gcm_client_for_testing() : NULL;
}

bool GCMDriverDesktop::IsStarted() const {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
  return gcm_started_;
}

bool GCMDriverDesktop::IsConnected() const {
  return connected_;
}

void GCMDriverDesktop::GetGCMStatistics(
    const GetGCMStatisticsCallback& callback,
    bool clear_logs) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
  DCHECK(!callback.is_null());

  request_gcm_statistics_callback_ = callback;
  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::GetGCMStatistics,
                 base::Unretained(io_worker_.get()),
                 clear_logs));
}

void GCMDriverDesktop::SetGCMRecording(const GetGCMStatisticsCallback& callback,
                                       bool recording) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  request_gcm_statistics_callback_ = callback;
  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::SetGCMRecording,
                 base::Unretained(io_worker_.get()),
                 recording));
}

void GCMDriverDesktop::UpdateAccountMapping(
    const AccountMapping& account_mapping) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::UpdateAccountMapping,
                 base::Unretained(io_worker_.get()),
                 account_mapping));
}

void GCMDriverDesktop::RemoveAccountMapping(const std::string& account_id) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::RemoveAccountMapping,
                 base::Unretained(io_worker_.get()),
                 account_id));
}

base::Time GCMDriverDesktop::GetLastTokenFetchTime() {
  return last_token_fetch_time_;
}

void GCMDriverDesktop::SetLastTokenFetchTime(const base::Time& time) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  last_token_fetch_time_ = time;

  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::SetLastTokenFetchTime,
                 base::Unretained(io_worker_.get()),
                 time));
}

void GCMDriverDesktop::WakeFromSuspendForHeartbeat(bool wake) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  wake_from_suspend_enabled_ = wake;

  // The GCM service has not been initialized.
  if (!delayed_task_controller_)
    return;

  if (!delayed_task_controller_->CanRunTaskWithoutDelay()) {
    // The GCM service was initialized but has not started yet.
    delayed_task_controller_->AddTask(
        base::Bind(&GCMDriverDesktop::WakeFromSuspendForHeartbeat,
                   weak_ptr_factory_.GetWeakPtr(),
                   wake_from_suspend_enabled_));
    return;
  }

  // The GCMClient is ready so we can go ahead and post this task to the
  // IOWorker.
  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::WakeFromSuspendForHeartbeat,
                 base::Unretained(io_worker_.get()),
                 wake_from_suspend_enabled_));
}

void GCMDriverDesktop::SetAccountTokens(
    const std::vector<GCMClient::AccountTokenInfo>& account_tokens) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  account_mapper_->SetAccountTokens(account_tokens);

  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::SetAccountTokens,
                 base::Unretained(io_worker_.get()),
                 account_tokens));
}

GCMClient::Result GCMDriverDesktop::EnsureStarted(
    GCMClient::StartMode start_mode) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  if (gcm_started_)
    return GCMClient::SUCCESS;

  // Have any app requested the service?
  if (app_handlers().empty())
    return GCMClient::UNKNOWN_ERROR;

  // Polling for channel status should be invoked when GCM is being requested,
  // no matter whether GCM is enabled or nor.
  gcm_channel_status_syncer_->EnsureStarted();

  if (!gcm_enabled_)
    return GCMClient::GCM_DISABLED;

  if (!delayed_task_controller_)
    delayed_task_controller_.reset(new GCMDelayedTaskController);

  // Note that we need to pass weak pointer again since the existing weak
  // pointer in IOWorker might have been invalidated when GCM is stopped.
  io_thread_->PostTask(
      FROM_HERE,
      base::Bind(&GCMDriverDesktop::IOWorker::Start,
                 base::Unretained(io_worker_.get()),
                 start_mode,
                 weak_ptr_factory_.GetWeakPtr()));

  return GCMClient::SUCCESS;
}

void GCMDriverDesktop::RemoveCachedData() {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());
  // Remove all the queued tasks since they no longer make sense after
  // GCM service is stopped.
  weak_ptr_factory_.InvalidateWeakPtrs();

  gcm_started_ = false;
  delayed_task_controller_.reset();
  ClearCallbacks();
}

void GCMDriverDesktop::MessageReceived(
    const std::string& app_id,
    const GCMClient::IncomingMessage& message) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  // Drop the event if the service has been stopped.
  if (!gcm_started_)
    return;

  GetAppHandler(app_id)->OnMessage(app_id, message);
}

void GCMDriverDesktop::MessagesDeleted(const std::string& app_id) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  // Drop the event if the service has been stopped.
  if (!gcm_started_)
    return;

  GetAppHandler(app_id)->OnMessagesDeleted(app_id);
}

void GCMDriverDesktop::MessageSendError(
    const std::string& app_id,
    const GCMClient::SendErrorDetails& send_error_details) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  // Drop the event if the service has been stopped.
  if (!gcm_started_)
    return;

  GetAppHandler(app_id)->OnSendError(app_id, send_error_details);
}

void GCMDriverDesktop::SendAcknowledged(const std::string& app_id,
                                        const std::string& message_id) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  // Drop the event if the service has been stopped.
  if (!gcm_started_)
    return;

  GetAppHandler(app_id)->OnSendAcknowledged(app_id, message_id);
}

void GCMDriverDesktop::GCMClientReady(
    const std::vector<AccountMapping>& account_mappings,
    const base::Time& last_token_fetch_time) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  UMA_HISTOGRAM_BOOLEAN("GCM.UserSignedIn", signed_in_);

  gcm_started_ = true;
  if (wake_from_suspend_enabled_)
    WakeFromSuspendForHeartbeat(wake_from_suspend_enabled_);

  last_token_fetch_time_ = last_token_fetch_time;

  GCMDriver::AddAppHandler(kGCMAccountMapperAppId, account_mapper_.get());
  account_mapper_->Initialize(account_mappings);

  delayed_task_controller_->SetReady();
}

void GCMDriverDesktop::OnConnected(const net::IPEndPoint& ip_endpoint) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  connected_ = true;

  // Drop the event if the service has been stopped.
  if (!gcm_started_)
    return;

  FOR_EACH_OBSERVER(GCMConnectionObserver,
                    connection_observer_list_,
                    OnConnected(ip_endpoint));
}

void GCMDriverDesktop::OnDisconnected() {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  connected_ = false;

  // Drop the event if the service has been stopped.
  if (!gcm_started_)
    return;

  FOR_EACH_OBSERVER(
      GCMConnectionObserver, connection_observer_list_, OnDisconnected());
}

void GCMDriverDesktop::GetGCMStatisticsFinished(
    const GCMClient::GCMStatistics& stats) {
  DCHECK(ui_thread_->RunsTasksOnCurrentThread());

  // Normally request_gcm_statistics_callback_ would not be null.
  if (!request_gcm_statistics_callback_.is_null())
    request_gcm_statistics_callback_.Run(stats);
  else
    LOG(WARNING) << "request_gcm_statistics_callback_ is NULL.";
}

}  // namespace gcm

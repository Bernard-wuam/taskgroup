#pragma once

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/append.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/consign.hpp>
#include <boost/asio/default_completion_token.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <list>
#include <mutex>
#include <utility>

class TaskGroup {
  boost::asio::steady_timer m_timer;
  std::list<boost::asio::cancellation_signal> m_cancellationSignal;
  std::mutex m_mutex;

public:
  TaskGroup(boost::asio::any_io_executor ioc)
      : m_timer{ioc, boost::asio::steady_timer::time_point::max()} {}

  template <typename CompletionToken = boost::asio::default_completion_token_t<
                boost::asio::any_io_executor>>
  auto adapt(CompletionToken &&completionToken) {
    using cancellationIteratorType = decltype(m_cancellationSignal)::iterator;

    class Remover {
      // the object here.
      TaskGroup *m_taskGroup;
      cancellationIteratorType m_cancellationIterator;

    public:
      Remover(TaskGroup *taskGroup,
              cancellationIteratorType cancellationIterator) noexcept
          : m_taskGroup(taskGroup),
            m_cancellationIterator(cancellationIterator) {

            };

      Remover(Remover &&others)
          : m_taskGroup(std::exchange(others.m_taskGroup, nullptr)),
            m_cancellationIterator(others.m_cancellationIterator) {};

      ~Remover() {
        if (m_taskGroup) {
          std::lock_guard(m_taskGroup->m_mutex);
          auto isLastIteterator =
              m_taskGroup->m_cancellationSignal.erase(m_cancellationIterator);
          if (isLastIteterator == m_taskGroup->m_cancellationSignal.end())
            m_taskGroup->m_timer.cancel();
        };
      }
    };

    auto cancellationSignal =
        m_cancellationSignal.emplace(m_cancellationSignal.end());

    return boost::asio::bind_cancellation_slot(
        cancellationSignal->slot(),
        boost::asio::consign(std::forward<CompletionToken>(completionToken),
                             Remover{this, cancellationSignal}));
  }

  template <
      typename CompletionHandler =
          boost::asio::default_completion_token_t<boost::asio::any_io_executor>>
  auto asyn_wait(CompletionHandler &&completionHandler =
                     boost::asio::default_completion_token_t<
                         boost::asio::any_io_executor>{}) {
    return boost::asio::async_compose<CompletionHandler,
                                      void(boost::system::error_code ec)>(
        [schedule = false, this](auto &&self,
                                 boost::system::error_code ec = {}) mutable {
          if (!schedule) {
            self.reset_cancellation_state(
                boost::asio::enable_total_cancellation());
           // schedule = false;
          }

          if (self.cancelled() != boost::asio::cancellation_type::none &&
              ec == boost::asio::error::operation_aborted) {

            ec = {};
          }

          {
            std::lock_guard lg(m_mutex);
            if (!m_cancellationSignal.empty() && !ec) {
              schedule = true;
              return m_timer.async_wait(std::move(self));
            }
          }

          if (!std::exchange(schedule, true))
            return boost::asio::post(boost::asio::append(std::move(self), ec));

          self.complete(ec);
        },
        completionHandler, m_timer);
  }

  void sendSignal(boost::asio::cancellation_type cancellationType)  {
    std::lock_guard ls(m_mutex);
    for (auto &f : m_cancellationSignal) {
      f.emit(cancellationType);
    }
  }
};

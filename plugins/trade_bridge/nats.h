#pragma once

#include <memory>
#include <string>

#include <tl/expected.hpp>
#include <nats/nats.h>

namespace nats
{
	template<typename Marshaler>
	class server
	{
		struct nats_conn_deleter
		{
			void operator() (natsConnection* conn) const noexcept
			{
				if (conn)
				{
					natsConnection_Drain(conn);
					natsConnection_FlushTimeout(conn, 1000);
					natsConnection_Destroy(conn);
				}
			}
		};
		using nats_conn_t = std::unique_ptr<natsConnection, nats_conn_deleter>;

		struct nats_subscr_deleter
		{
			void operator() (natsSubscription* sub) const noexcept
			{
				if (sub)
				{
					natsSubscription_Drain(sub);
					natsSubscription_WaitForDrainCompletion(sub, 1000);
					natsSubscription_Destroy(sub);
				}
			}
		};
		using nats_subscr_t = std::shared_ptr<natsSubscription>;

		using nats_msg_t = std::unique_ptr<natsMsg, decltype(&natsMsg_Destroy)>;

		using subscription_read_message_error_t = tl::expected<std::string, std::string>;
		template<typename Message>
		using subscription_read_message_result_t = tl::expected<Message, subscription_read_message_error_t>;
		template<typename Message>
		using subscription_callback_t = std::function<subscription_read_message_result_t<Message>()>;

	public:
		server()
			: m_connection{ nullptr, nats_conn_deleter{} }
		{
		}

		tl::expected<void, std::string> connect(const std::string_view url)
		{
			natsConnection* raw_conn{ nullptr };
			natsStatus stat = natsConnection_ConnectTo(&raw_conn, url.data());
			if (stat != NATS_OK)
			{
				return tl::unexpected<std::string>(natsStatus_GetText(stat));
			}
			m_connection.reset(raw_conn);
			return {};
		}

		template<typename Message>
		tl::expected<void, std::string> publish(const std::string_view topic_name, Message&& message)
		{
			natsStatus stat = natsConnection_PublishString(m_connection.get(), topic_name.data(),
				Marshaler::marshal(std::forward<Message>(message)).data());
			if (stat != NATS_OK)
			{
				return tl::unexpected<std::string>(natsStatus_GetText(stat));
			}
			return {};
		}

		template<typename Message>
		tl::expected<subscription_callback_t<Message>, std::string> subscribe_sync(const std::string_view topic_name)
		{
			natsSubscription* raw_sub = nullptr;
			if (auto status = natsConnection_SubscribeSync(&raw_sub, m_connection.get(), topic_name.data()); status != NATS_OK)
			{
				return tl::unexpected<std::string>(natsStatus_GetText(status));
			}
			nats_subscr_t sub{ raw_sub, nats_subscr_deleter{} };
			if (auto status = natsSubscription_SetPendingLimits(sub.get(), 1024, 8 * 1024 * 1024); status != NATS_OK)
			{
				return tl::unexpected<std::string>(natsStatus_GetText(status));
			}

            return [sub, topic_name = std::string(topic_name)]() mutable -> subscription_read_message_result_t<Message>
            {
                natsMsg* raw_msg = nullptr;
                natsStatus status = natsSubscription_NextMsg(&raw_msg, sub.get(), 2000);
                nats_msg_t msg{ raw_msg, natsMsg_Destroy };
                switch (status)
                {
                case NATS_TIMEOUT:
					return tl::unexpected{ "Timeout waiting for message on topic: " + topic_name };
                case NATS_CONNECTION_CLOSED:
                    return tl::unexpected{ "Connection subscription closed for topic: " + topic_name };

                case NATS_INVALID_SUBSCRIPTION:
                    return tl::unexpected{ subscription_read_message_error_t{ tl::unexpected{ "Invalid subscription for topic: " + topic_name } } };
				case NATS_OK:
					const auto data = natsMsg_GetData(msg.get());
					auto result = Marshaler::template unmarshal<Message>(data);
					if (result)
					{
						return *result;
					}

					return tl::unexpected{ subscription_read_message_error_t{ tl::unexpected{ "Failed to unmarshal message from topic: " + topic_name + ", data: " + data + " error: " + result.error() } } };
                }
				return tl::unexpected{ subscription_read_message_error_t{ tl::unexpected{ "Unknown error while reading message from topic: " + topic_name } } };
            };
		}

	private:
		nats_conn_t		m_connection;
	};
}

#pragma once

#include <string>
#ifndef __cplusplus
#error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <queue>
#include <string.h>
#include <vector>

using namespace eprosima::fastdds::dds;

/**
 * @brief The class to subscriber vehicle info
 */
template<class T, class TPubSubType>
class DDSGenericSubscriber {
  public:
    DDSGenericSubscriber();
    DDSGenericSubscriber(std::string& topic_name);
    ~DDSGenericSubscriber();
    bool Init(std::function<void(T*)> cb_);

  private:
    DomainParticipant *participant_;
    Subscriber *subscriber_;
    Topic *topic_;
    DataReader *reader_;
    TypeSupport type_;

    class SubListener : public DataReaderListener {
      public:
        SubListener() = default;
        ~SubListener() override = default;
        void
        on_data_available(DataReader *reader) override;
        void on_subscription_matched(
            DataReader *reader,
            const SubscriptionMatchedStatus &info)
            override;

        std::function<void(T*)> process_callback_;
        // std::queue<T> queue;
        // std::mutex* mtx_;

        int matched = 0;
    } listener_;

    std::string topic_name_;
};


template<class T, class TPubSubType>
DDSGenericSubscriber<T, TPubSubType>::DDSGenericSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , reader_(nullptr)
    , topic_(nullptr)
    , type_(new TPubSubType()){
    topic_name_ = type_.get_type_name();
    // do nothing
}

template<class T, class TPubSubType>
DDSGenericSubscriber<T, TPubSubType>::DDSGenericSubscriber(std::string& topic_name)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , reader_(nullptr)
    , topic_(nullptr)
    , type_(new TPubSubType())
    , topic_name_(topic_name){
    // do nothing
}

template<class T, class TPubSubType>
DDSGenericSubscriber<T, TPubSubType>::~DDSGenericSubscriber() {
    
    if (reader_ != nullptr) {
        subscriber_->delete_datareader(reader_);
    }

    if (topic_ != nullptr) {
        participant_->delete_topic(topic_);
    }

    if (subscriber_ != nullptr) {
        participant_->delete_subscriber(subscriber_);
    }

    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

template<class T, class TPubSubType>
bool DDSGenericSubscriber<T, TPubSubType>::Init(std::function<void(T*)> func) {

    // Create the participant
    DomainParticipantQos pqos;
    pqos.name("Participant");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (nullptr == participant_) {
        return false;
    }

    // Create the subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (nullptr == subscriber_) {
        return false;
    }

    type_.register_type(participant_);
    printf("Create Subscriber, topic_name = %s, type_name = %s\n", topic_name_.c_str(), type_.get_type_name().c_str());
    topic_ = participant_->create_topic(topic_name_, type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (nullptr == topic_) {
        return false;
    }

    reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
    if (nullptr == reader_) {
        return false;
    }
    listener_.process_callback_ = func;

    std::string log_info = type_.get_type_name() + " datareader created";
    return true;
}

template<class T, class TPubSubType>
void DDSGenericSubscriber<T, TPubSubType>::SubListener::on_subscription_matched(DataReader* reader,
                                                                const SubscriptionMatchedStatus& info) {
    if (1 == info.current_count_change) {
    // Debug
    #ifdef DBG
        printf("matched dataReader: %s, total_count = %d\n", reader->type().get_type_name().c_str(),
            info.total_count);
    #endif
    } else if (-1 == info.current_count_change) {
    // Debug
    #ifdef DBG
        printf("unmatched dataReader: %s, total_count = %d\n", reader->type().get_type_name().c_str(),
            info.total_count);
    #endif
    } else {
        std::string suffix = "invalid value for PublicationMatchedStatus current count change";
        std::string log_info = std::to_string(info.current_count_change) + suffix;
    }
}

// TODO: use hash_map and switch-case will be more efficient than if-else
template<class T, class TPubSubType>
void DDSGenericSubscriber<T, TPubSubType>::SubListener::on_data_available(DataReader* reader) {
    SampleInfo info;
    T msg;
    if (reader->take_next_sample(&msg, &info) == ReturnCode_t::RETCODE_OK) {
        if (info.valid_data) {
            process_callback_(&msg);
        }
    }
}

/* EOF */

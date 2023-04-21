#pragma once

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#include <chrono>
#include <string>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>


using namespace eprosima::fastdds::dds;

template <class T, class TPubSubType>
class DDSGenericPublisher
{
public:

    // Constructor
    DDSGenericPublisher();
    DDSGenericPublisher(std::string& topic_name);

    // Destructor
    ~DDSGenericPublisher();
    bool Init();
    void Publish(T* msg);

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::TypeSupport type_;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener {
    public:
        PubListener() = default;
        ~PubListener() = default;
        void on_publication_matched(eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;
    } listener_;

    std::string topic_name_;
};

template<class T, class TPubSubType>
DDSGenericPublisher<T, TPubSubType>::DDSGenericPublisher()
    : participant_(nullptr)
    , topic_(nullptr)
    , publisher_(nullptr)
    , writer_(nullptr)
    , type_(new TPubSubType()){
    topic_name_ = type_.get_type_name();
    this->Init();
    // do nothing
}

template<class T, class TPubSubType>
DDSGenericPublisher<T, TPubSubType>::DDSGenericPublisher(std::string& topic_name)
    : participant_(nullptr)
    , topic_(nullptr)
    , publisher_(nullptr)
    , writer_(nullptr)
    , type_(new TPubSubType())
    , topic_name_(topic_name){
    this->Init();
    // do nothing
}

template<class T, class TPubSubType>
DDSGenericPublisher<T, TPubSubType>::~DDSGenericPublisher() {
    // Destruct dds func
    if (!writer_) {
        publisher_->delete_datawriter(writer_);
    }

    if (!publisher_) {
        participant_->delete_publisher(publisher_);
    }

    if (!topic_) {
        participant_->delete_topic(topic_);
    }

    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

template<class T, class TPubSubType>
bool DDSGenericPublisher<T, TPubSubType>::Init() {

    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (nullptr == participant_) {
        // DLT_LOG(PtpCom, DLT_LOG_ERROR, DLT_CSTRING("Create participant failed"));
        return false;
    }

    // Create the publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (nullptr == publisher_) {
        // DLT_LOG(PtpCom, DLT_LOG_ERROR, DLT_CSTRING("Create participant failed"));
        return false;
    }

    type_.register_type(participant_);
    printf("Create Publisher, topic_name = %s, type_name = %s\n", topic_name_.c_str(), type_.get_type_name().c_str());
    topic_ = participant_->create_topic(topic_name_, type_.get_type_name(), TOPIC_QOS_DEFAULT);

    if (nullptr == topic_) {
        // DLT_LOG(PtpCom, DLT_LOG_ERROR, DLT_CSTRING("Create participant failed"));
        return false;
    }

    writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);

    if (nullptr == writer_) {
        // DLT_LOG(PtpCom, DLT_LOG_ERROR, DLT_CSTRING("Create participant failed"));
        return false;
    }

    // std::string log_info = topic_->get_type_name() + " datawriter created";
    // DLT_LOG(PtpCom, DLT_LOG_INFO, DLT_CSTRING("log_info.c_str()"));
    return true;
}

template<class T, class TPubSubType>
void DDSGenericPublisher<T, TPubSubType>::PubListener::on_publication_matched(
    eprosima::fastdds::dds::DataWriter* writer, const eprosima::fastdds::dds::PublicationMatchedStatus& info) {
    if (1 == info.current_count_change) {
        // Debug
        #ifdef DBG
        printf("matched datawriter: %s, total_count = %d\n", writer->get_type().get_type_name().c_str(),
               info.total_count);
        #endif
        // DLT_LOG(PtpCom, DLT_LOG_INFO, DLT_CSTRING("DataWriter matched"));
    } else if (-1 == info.current_count_change) {
        // --total_matched_;
        // Debug
        #ifdef DBG
        printf("unmatched datawriter: %s, total_count = %d\n", writer->get_type().get_type_name().c_str(),
               info.total_count);
        #endif
        // DLT_LOG(PtpCom, DLT_LOG_INFO, DLT_CSTRING("DataWriter unmatched"));
    } else {
        std::string log_info = std::to_string(info.current_count_change) +
                               "invalid value for PublicationMatchedStatus current count change";
        // DLT_LOG(PtpCom, DLT_LOG_WARN, DLT_CSTRING(log_info.c_str()));
    }
}

template<class T, class TPubSubType>
void DDSGenericPublisher<T, TPubSubType>::Publish(T* msg) {
    writer_->write(msg);
}
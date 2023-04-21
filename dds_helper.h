#pragma once


#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <map>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>


using namespace eprosima::fastdds::dds;


class DDSHelper
{
public:
    DDSHelper();
    ~DDSHelper();

    bool registe_writer(const std::string& topic_name,  TopicDataType* data_type);

    // bool registe_reader(const std::string& topic_name,  callbackFunc* func);

    void Publish(std::string topic, void* msg);

private:
    bool Init();

private:
    DomainParticipant* participant_;
    Publisher* publisher_;
    std::map<std::string, DataWriter*> writer_map_;
    std::vector<Topic*> topic_list_;
    std::vector<TypeSupport> type_list_;;

    class PubListener : public DataWriterListener {
    public:
        PubListener() = default;
        ~PubListener() = default;
        void on_publication_matched(DataWriter* writer,
            const PublicationMatchedStatus& info) override;
    } listener_;
};


DDSHelper::DDSHelper()
    : participant_(nullptr)
    , publisher_(nullptr)
{
    this->Init();
}


DDSHelper::~DDSHelper() 
{
    // Destruct dds func
    // if (!writer_) {
    //     publisher_->delete_datawriter(writer_);
    // }
    if (!publisher_) {
        participant_->delete_publisher(publisher_);
    }
    // if (!topic_) {
    //     participant_->delete_topic(topic_);
    // }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}


bool DDSHelper::Init() 
{

    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (nullptr == participant_) {
        return false;
    }

    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (nullptr == publisher_) {
        return false;
    }
    return true;
}


bool DDSHelper::registe_writer(const std::string& topic_name,  TopicDataType* data_type)
{
    TypeSupport new_type(data_type);
    new_type.register_type(participant_);
    type_list_.push_back(new_type);

    printf("Create Publisher, topic_name = %s, type_name = %s\n", topic_name.c_str(), new_type.get_type_name().c_str());
    eprosima::fastdds::dds::Topic* new_topic = participant_->create_topic(topic_name, new_type.get_type_name(), TOPIC_QOS_DEFAULT);
    if (nullptr == new_topic) {
        return false;
    }
    topic_list_.push_back(new_topic);

    auto new_writer = publisher_->create_datawriter(new_topic, DATAWRITER_QOS_DEFAULT, &listener_);
    if (nullptr == new_writer) {
        return false;
    }
    writer_map_[topic_name]=new_writer;
    
    return true;
}



void DDSHelper::PubListener::on_publication_matched(
    DataWriter* writer, const PublicationMatchedStatus& info) 
{
    if (1 == info.current_count_change) {
        printf("matched datawriter: %s, total_count = %d\n", writer->get_type().get_type_name().c_str(),
               info.total_count);
    } else if (-1 == info.current_count_change) {
        printf("unmatched datawriter: %s, total_count = %d\n", writer->get_type().get_type_name().c_str(),
               info.total_count);
    } else {
        std::string log_info = std::to_string(info.current_count_change) +
                               "invalid value for PublicationMatchedStatus current count change";
    }
}


void DDSHelper::Publish(std::string topic, void* msg) 
{
    if (writer_map_.find(topic) != writer_map_.end()){
        writer_map_[topic]->write(msg);
    }
}
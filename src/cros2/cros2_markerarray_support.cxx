/** ==================================================================
 * FILE: cros2_markerarray_support.cxx
 * Class for interfacing a native Connext DDS application with the 
 *  ROS 2 visualization_msgs::msg::dds_::MarkerArray_ data type.
 * This program uses the C++11 option of RTIDDSGen, using the type
 *  support source code generated by the command line:
 *   rtiddsgen -language C++11 <typedef-file.idl>
 **/
#include "cros2_markerarray_support.hpp"

/*ci
 * \brief cros2markerarray class constructor
 *
 * \details
 * Instantiate a visualization_msgs::msg::dds_::MarkerArray_ publisher
 * and/or subscriber with supporting writer, reader, topic, etc.
 *
 * \param[in]    markerarrayName   Name of the DDS topic
 * \param[in]    opt_en       bitfield of options (pub, sub) to enable
 * \param[in]    participant  DDS participant, already created
 */
cros2markerarray::cros2markerarray(const std::string markerarrayName, uint32_t opt_en, dds::domain::DomainParticipant participant) :
    data_topic(dds::core::null), pub_sample(),
    data_wr(dds::core::null), data_rd(dds::core::null),
    waitset()
{
    // create topic (pub or sub)
    const std::string data_topname("rt/" + markerarrayName);
    data_topic = dds::topic::Topic<visualization_msgs::msg::dds_::MarkerArray_>(participant, data_topname);

    if (opt_en & (1 << CROS2_PUB_ON)) {
        // DataWriter
        data_wr = dds::pub::DataWriter<visualization_msgs::msg::dds_::MarkerArray_>(dds::pub::Publisher(participant), data_topic);
    }

    if (opt_en & (1 << CROS2_SUB_ON)) {
        // DataReader, ReadCondition, WaitSet
        data_rd = dds::sub::DataReader<visualization_msgs::msg::dds_::MarkerArray_>(dds::sub::Subscriber(participant), data_topic);
        dds::sub::cond::ReadCondition data_readCond(
            data_rd, dds::sub::status::DataState::any(),
            [this](/* dds::core::cond::Condition condition */)
        {
            data_rcv_count += rcv_dataProc(data_rd);
        }
        );
        waitset += data_readCond;
    }
}

// Receive data handler
int cros2markerarray::rcv_dataProc(dds::sub::DataReader<visualization_msgs::msg::dds_::MarkerArray_>& reader)
{
    int count = 0;
    dds::sub::LoanedSamples<visualization_msgs::msg::dds_::MarkerArray_> samples = reader.take();
    for (const auto& sample : samples) {
        if (sample.info().valid()) {
            count++;
            //std::cout << "add " << sample.data() << std::endl;
        }
    }
    return count;
}


/*ci
 * \brief Publish the MarkerArray_ topic.
 *
 * \details
 * Timestamp and publish the MarkerArray_ topic
 *
 */
void cros2markerarray::publish(void)
{ 
    timespec ts;
    tstamp_get(&ts);
    for (int i = 0; i < pub_sample.markers().size(); i++) {
        pub_sample.markers().at(i).header().stamp().sec((int32_t)ts.tv_sec);
        pub_sample.markers().at(i).header().stamp().nanosec(ts.tv_nsec);
    }
    data_wr.write(pub_sample);
}


// destructor
cros2markerarray::~cros2markerarray() {}

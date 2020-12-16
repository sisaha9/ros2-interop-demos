/** ==================================================================
 * FILE: cros2_polygonstamped_support.cxx
 * Class for interfacing a native Connext DDS application with the 
 *  ROS 2 geometry_msgs::msg::dds_::PolygonStamped_ data type.
 * This program uses the C++11 option of RTIDDSGen, using the type
 *  support source code generated by the command line:
 *   rtiddsgen -language C++11 <typedef-file.idl>
 **/
#include "cros2_polygonstamped_support.hpp"

 /*ci
  * \brief cros2polygonstamped class constructor
  *
  * \details
  * Instantiate a geometry_msgs::msg::dds_::PolygonStamped_ publisher
  * and/or subscriber with supporting writer, reader, topic, etc.
  *
  * \param[in]    polygonstampedName    Name of the DDS topic
  * \param[in]    opt_en       bitfield of options (pub, sub) to enable
  * \param[in]    participant  DDS participant, already created
  */
cros2polygonstamped::cros2polygonstamped(const std::string polygonstampedName, uint32_t opt_en, dds::domain::DomainParticipant participant,
    int(*rcv_fptr)(dds::sub::DataReader<geometry_msgs::msg::dds_::PolygonStamped_>)) :
    data_topic(dds::core::null), pub_sample(),
    data_wr(dds::core::null), data_rd(dds::core::null),
    waitset()
{
    // create topics (pub or sub)
    const std::string data_topname("rt/" + polygonstampedName);
    data_topic = dds::topic::Topic<geometry_msgs::msg::dds_::PolygonStamped_>(participant, data_topname);

    if (opt_en & (1 << CROS2_PUB_ON)) {
        // DataWriter
        data_wr = dds::pub::DataWriter<geometry_msgs::msg::dds_::PolygonStamped_>(dds::pub::Publisher(participant), data_topic);
    }

    if (opt_en & (1 << CROS2_SUB_ON)) {
        // DataReader, ReadCondition, WaitSet
        data_rd = dds::sub::DataReader<geometry_msgs::msg::dds_::PolygonStamped_>(dds::sub::Subscriber(participant), data_topic);
        dds::sub::cond::ReadCondition data_readCond(
            data_rd, dds::sub::status::DataState::any(),
            [rcv_fptr, this](/* dds::core::cond::Condition condition */)
        {
            if (rcv_fptr == NULL) {
                // use the default receive handler if no other was specified
                data_rcv_count += rcv_dataProc(data_rd);
            }
            else {
                // use the specified receive handler
                data_rcv_count += rcv_fptr(data_rd);
            }
        }
        );
        waitset += data_readCond;
    }
}

// Receive data handler
int cros2polygonstamped::rcv_dataProc(dds::sub::DataReader<geometry_msgs::msg::dds_::PolygonStamped_>& reader)
{
    int count = 0;
    dds::sub::LoanedSamples<geometry_msgs::msg::dds_::PolygonStamped_> samples = reader.take();
    for (const auto& sample : samples) {
        if (sample.info().valid()) {
            count++;
            //std::cout << "add " << sample.data() << std::endl;
        }
    }
    return count;
}


/*ci
 * \brief Publish the PolygonStamped_ topic.
 *
 * \details
 * Timestamp and publish the PolygonStamped_ topic
 *
 */
void cros2polygonstamped::publish(void)
{ 
    timespec ts;
    tstamp_get(&ts);
    pub_sample_header_tstamp_set((int32_t)ts.tv_sec, ts.tv_nsec);
    data_wr.write(pub_sample); 
}

// destructor
cros2polygonstamped::~cros2polygonstamped() {}

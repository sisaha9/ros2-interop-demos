/** ==================================================================
 * FILE: cros2_pointcloud_support.cxx
 * Class for interfacing a native Connext DDS application with the 
 *  ROS 2 sensor_msgs::msg::dds_::PointCloud2_ data type.
 * This program uses the C++11 option of RTIDDSGen, using the type
 *  support source code generated by the command line:
 *   rtiddsgen -language C++11 <typedef-file.idl>
 **/
#include "cros2_pointcloud_support.hpp"

 /*ci
  * \brief cros2pointcloud class constructor
  *
  * \details
  * Instantiate a sensor_msgs::msg::dds_::PointCloud2_ publisher
  * and/or subscriber with supporting writer, reader, topic, etc.
  *
  * \param[in]    cloudName    Name of the DDS topic
  * \param[in]    opt_en       bitfield of options (pub, sub) to enable
  * \param[in]    participant  DDS participant, already created
  */
cros2pointcloud::cros2pointcloud(const std::string cloudName, uint32_t opt_en, dds::domain::DomainParticipant participant, 
    int(*rcv_fptr)(dds::sub::DataReader<sensor_msgs::msg::dds_::PointCloud2_>))
    :
    data_topic(dds::core::null), pub_sample(),
    data_wr(dds::core::null), data_rd(dds::core::null),
    waitset()
{
    // create topics
    const std::string data_topname("rt/" + cloudName);
    data_topic = dds::topic::Topic<sensor_msgs::msg::dds_::PointCloud2_>(participant, data_topname);

    if (opt_en & (1 << CROS2_PUB_ON)) {
        // DataWriter
#if 0   // bounded
        data_wr = dds::pub::DataWriter<sensor_msgs::msg::dds_::PointCloud2_>(dds::pub::Publisher(participant), data_topic);
#else   // unbounded or bounded support
        dds::pub::qos::DataWriterQos writer_qos = dds::core::QosProvider::Default().datawriter_qos();
        writer_qos.policy<rti::core::policy::Property>().set({ "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size", "1048576" });
        data_wr = dds::pub::DataWriter<sensor_msgs::msg::dds_::PointCloud2_>(dds::pub::Publisher(participant), data_topic, writer_qos);
#endif
    }

    if (opt_en & (1 << CROS2_SUB_ON)) {
        // DataReader, ReadCondition, WaitSet
        data_rd = dds::sub::DataReader<sensor_msgs::msg::dds_::PointCloud2_>(dds::sub::Subscriber(participant), data_topic);
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
        data_rcv_count = 0;
    }
}

// Receive data handler
int cros2pointcloud::rcv_dataProc(dds::sub::DataReader<sensor_msgs::msg::dds_::PointCloud2_ >& reader)
{
    int count = 0;
    dds::sub::LoanedSamples<sensor_msgs::msg::dds_::PointCloud2_> samples = reader.take();
    for (const auto& sample : samples) {
        if (sample.info().valid()) {
            //std::cout << "PtC2 " << sample.data() << std::endl;
            count++;
        }
    }
    //std::cout <<  std::endl;
    return count;
}

/*ci
 * \brief Publish the PointCloud2_ topic.
 *
 * \details
 * Timestamp and publish the PointCloud2_ topic
 *
 */
void cros2pointcloud::publish(void)
{ 
    timespec ts;
    tstamp_get(&ts);
    pub_sample.header().stamp().sec((int32_t)ts.tv_sec);
    pub_sample.header().stamp().nanosec(ts.tv_nsec);
    data_wr.write(pub_sample);
}


/** --------------------------------------------------------------
 * pub_sample_config_fields()
 * Configure the header for a given profile
 **/
void cros2pointcloud::pub_sample_config_fields(ptc2_data_fmt myFormat)
{
    switch (myFormat) {         // FIXME: everything defaults to one format.  Add more.
    case PTCLOUD2_FMT_XYZ_RGB8_FLOAT32:
    default:
        // init the sample (use a static X|Y|Z|RGB with float32 values config for now)
        pub_sample.fields().resize(4);
        pub_sample.fields().at(0).name("x");
        pub_sample.fields().at(0).offset(0);
        pub_sample.fields().at(0).datatype(7);  // 2=UINT8, 7=float32
        pub_sample.fields().at(0).count(1);
        pub_sample.fields().at(1).name("y");
        pub_sample.fields().at(1).offset(4);
        pub_sample.fields().at(1).datatype(7);
        pub_sample.fields().at(1).count(1);
        pub_sample.fields().at(2).name("z");
        pub_sample.fields().at(2).offset(8);
        pub_sample.fields().at(2).datatype(7);
        pub_sample.fields().at(2).count(1);
        pub_sample.fields().at(3).name("rgb");
        pub_sample.fields().at(3).offset(12);
        pub_sample.fields().at(3).datatype(7);
        pub_sample.fields().at(3).count(1);
        bytesPerPoint = 16;     // 4 bytes per float * 4 values
        break;
    }
}

// destructor
cros2pointcloud::~cros2pointcloud() {}

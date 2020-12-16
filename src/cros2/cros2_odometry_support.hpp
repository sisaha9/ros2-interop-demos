/** ==================================================================
 * cros2_odometry_support.hpp
 * Class for interfacing a native Connext DDS application with the 
 *  ROS 2 nav_msgs::msg::dds_::Odometry_ data type.
 * This program uses the C++11 option of RTIDDSGen, using the type
 *  support source code generated by the command line:
 *   rtiddsgen -language C++11 <arch> cros2_types.idl
 **/

#ifndef __CROS2_ODOMETRY_SUPPORT_HPP__
#define __CROS2_ODOMETRY_SUPPORT_HPP__
#include "cros2_common.hpp"             // common includes and defs
#include "nav_msgs/msg/OdometryPlugin.hpp"

/** ----------------------------------------------------------------
 * cros2odometry class
 * This class supports ROS2 odometry
 * ------------------------------------------------------------- **/
class cros2odometry
{
public:
    cros2odometry(const std::string odometryName, uint32_t opt_en, dds::domain::DomainParticipant participant,
        int(*rcv_fptr)(dds::sub::DataReader<nav_msgs::msg::dds_::Odometry_>) = NULL);
    ~cros2odometry();

    void publish(void);
    nav_msgs::msg::dds_::Odometry_ *pub_sample_get_ptr(void) { return &pub_sample; }

    // odometry data sample accessors
    void pub_sample_header_tstamp_set(int32_t newSec, uint32_t newNanosec) { 
        pub_sample.header().stamp().sec(newSec); 
        pub_sample.header().stamp().nanosec(newNanosec);
    }
    void pub_sample_header_frameid_set(const std::string newId) { pub_sample.header().frame_id(newId); }
    void pub_sample_child_frame_id_set(const std::string newId) { pub_sample.child_frame_id(newId); }
    int data_rcv_count_get(void) { return data_rcv_count; }

private:
    // DDS receive data handler
    int rcv_dataProc(dds::sub::DataReader<nav_msgs::msg::dds_::Odometry_> & reader);

    // DDS Topic, DataWriter, DataReader, Sample(for publish)
    dds::topic::Topic<nav_msgs::msg::dds_::Odometry_> data_topic;
    dds::pub::DataWriter<nav_msgs::msg::dds_::Odometry_> data_wr;
    dds::sub::DataReader<nav_msgs::msg::dds_::Odometry_> data_rd;
    nav_msgs::msg::dds_::Odometry_ pub_sample;

    // misc
    dds::core::cond::WaitSet waitset;
    int data_rcv_count;
};
#endif  // ndef __CROS2_ODOMETRY_SUPPORT_HPP__
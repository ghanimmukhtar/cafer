//| This file is a part of the CAFER framework developped within
//| the DREAM project (http://www.robotsthatdream.eu/).
//| Copyright 2015, ISIR / Universite Pierre et Marie Curie (UPMC)
//| Main contributor(s):
//|   * Stephane Doncieux, stephane.doncieux@isir.upmc.fr
//|
//|
//| This experiment allows to generate neural networks for simple
//| navigation tasks (obstacle avoidance and maze navigation).
//|
//| This software is governed by the CeCILL license under French law
//| and abiding by the rules of distribution of free software.  You
//| can use, modify and/ or redistribute the software under the terms
//| of the CeCILL license as circulated by CEA, CNRS and INRIA at the
//| following URL "http://www.cecill.info".
//|
//| As a counterpart to the access to the source code and rights to
//| copy, modify and redistribute granted by the license, users are
//| provided only with a limited warranty and the software's author,
//| the holder of the economic rights, and the successive licensors
//| have only limited liability.
//|
//| In this respect, the user's attention is drawn to the risks
//| associated with loading, using, modifying and/or developing or
//| reproducing the software by the user in light of its specific
//| status of free software, that may mean that it is complicated to
//| manipulate, and that also therefore means that it is reserved for
//| developers and experienced professionals having in-depth computer
//| knowledge. Users are therefore encouraged to load and test the
//| software's suitability as regards their requirements in conditions
//| enabling the security of their systems and/or data to be ensured
//| and, more generally, to use and operate it in the same conditions
//| as regards security.
//|
//| The fact that you are presently reading this means that you have
//| had knowledge of the CeCILL license and that you accept its terms.

#include <ros/ros.h>
#include <gtest/gtest.h>

#include "cafer_core/cafer_core.hpp"

#include <std_msgs/Int64.h>
#include <ros/impl/duration.h>


class DummyClient : public cafer_core::Component {
    using cafer_core::Component::Component; // To inherit Component's constructor

    cafer_core::shared_ptr<ros::Publisher> dummy_p;
    long int n;

public:
    void client_connect_to_ros(void)
    {
        dummy_p.reset(new ros::Publisher(my_ros_nh->advertise<std_msgs::Int64>("dummy_topic", 10)));
        n = 0;
        _is_init = true;
    }

    ~DummyClient()
    { disconnect_from_ros(); }


    void client_disconnect_from_ros(void)
    {
        dummy_p.reset();
    }

    void init(void)
    { connect_to_ros(); }

    void update(void)
    { }

    void publish_data(void)
    {
        if (is_connected_to_ros()) {
            std_msgs::Int64 v;
            v.data = n;
            dummy_p->publish(v);
            n++;
        }
    }
};


int main(int argc, char **argv)
{

    cafer_core::init(argc, argv, "component_test_node");

    std::string management_topic;
    cafer_core::ros_nh->param("management_topic", management_topic, std::string("component_test_management"));
    double freq;
    cafer_core::ros_nh->param("frequency", freq, 40.0);

    ROS_WARN_STREAM("Management topic for test node: " << management_topic << " namespace: " <<
                    cafer_core::ros_nh->getNamespace());

    DummyClient cc(management_topic, "dummy_node", freq);

    cc.wait_for_init();

    while (ros::ok() && (!cc.get_terminate())) {
        cc.spin();
        cc.update();
        cc.sleep();
    }

    if (cc.get_terminate()) {
        cc.shutdown();
    }

    return 0;
}

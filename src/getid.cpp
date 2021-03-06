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

#include <string>
#include <unordered_map>
#include <std_msgs/String.h>

#include "ros/ros.h"
#include "cafer_core/GetID.h"

int main(int argc, char** argv)
{
//    std::unordered_map<std::string, uint32_t> map_ID;

    uint32_t id_counter = 0;

    auto id_generator = [&id_counter](cafer_core::GetID::Request& req, cafer_core::GetID::Response& res)
    {
//        if (map_ID.find(req.name) == map_ID.end()) {
//            map_ID.emplace(req.name, 0);
//        }
//        else {
//            map_ID[req.name]++;
//        }
        res.id = ++id_counter;

        ROS_INFO("ID request value received: %s", req.name.c_str());
        ROS_INFO("Assigned ID: %ld", static_cast<uint32_t>(res.id));

        return true;
    };

    ros::init(argc, argv, "getID_server");
    ros::NodeHandle n;

    auto service = n.advertiseService<cafer_core::GetID::Request, cafer_core::GetID::Response>("get_id", id_generator);
    ROS_INFO("Ready to provide new IDs for CAFER components.");
    ros::spin();

    return 0;
}

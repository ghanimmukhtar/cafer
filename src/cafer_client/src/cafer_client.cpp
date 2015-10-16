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

#include <boost/shared_ptr.hpp>
#include <ros/ros.h>
#include <ros/spinner.h>
#include "cafer_client.hpp"

#include <tbb/concurrent_hash_map.h>

namespace cafer_client {
  
  // Hashing for strings
  struct MyHashCompare {
    static size_t hash( const std::string& x ) {
      size_t h = 0;
      for( const char* s = x.c_str(); *s; ++s )
	h = (h*17)^*s;
      return h;
    }
    //! True if strings are equal
    static bool equal( const std::string& x, const std::string& y ) {
      return x.compare(y)==0;
    }
  };

  // set of node groups allocated to this sferes run (to clean it up at the end)
  tbb::concurrent_hash_map<std::string,std::string,MyHashCompare> allocated_node_group;



  boost::shared_ptr<ros::NodeHandle> ros_nh;
  float ros_frequency=30;


  void init(int argc, char **argv, std::string node_name, float frequency) {
    ros::init(argc, argv, node_name);		
    ros_nh.reset(new ros::NodeHandle);
    std::cout<<"Initialising ROS. Node name: "<<node_name<<" frequency: "<<frequency<<std::endl;
    ros_frequency=frequency;
  }	
  
  std::string get_node_group(std::string namespace_base, std::string launch_file) {
    std::cout<<"Trying to get a node: "<<namespace_base<<" launch file: "<<launch_file<<std::endl;
    cafer_server::LaunchNode v;
    v.request.namespace_base = namespace_base;
    v.request.launch_file = launch_file;
    v.request.frequency = ros_frequency;
    ros::ServiceClient client = ros_nh->serviceClient<cafer_server::LaunchNode>("/cafer/get_node_group");
    std::string ns="<Failed>";
    if (client.call(v))
      {
	ns=v.response.created_namespace;
	ROS_INFO("Obtained node(s): namespace=%s launch file=%s", ns.c_str(), launch_file.c_str());
	tbb::concurrent_hash_map<std::string,std::string,MyHashCompare>::accessor a;
	allocated_node_group.insert(a,ns);
	a->second=namespace_base;
      }
    else
      {
	ROS_ERROR("Failed to call service get_node_group");
      }
    return ns;
  }

  void release_node_group(std::string namespace_base, std::string gr_namespace) {
    cafer_server::ReleaseNode v;
    v.request.namespace_base=namespace_base;
    v.request. gr_namespace=gr_namespace;
    ros::ServiceClient client = ros_nh->serviceClient<cafer_server::ReleaseNode>("/cafer/release_node_group");
    std::string ns="<Failed>";
    if (client.call(v))
      {
	ROS_INFO("Released node group=%s: %s", gr_namespace.c_str(),v.response.ack?"OK":"KO");
      }
    else
      {
	ROS_ERROR("Failed to call service release_node_group");
      }

  }
  void kill_node_group(std::string namespace_base, std::string gr_namespace) {
    cafer_server::KillNodeGroup v;
    v.request.namespace_base=namespace_base;
    v.request.gr_namespace=gr_namespace;
    ros::ServiceClient client = ros_nh->serviceClient<cafer_server::KillNodeGroup>("/cafer/kill_node_group");
    std::string ns="<Failed>";
    if (client.call(v))
      {
	ROS_INFO("Kill node group=%s: %s", gr_namespace.c_str(),v.response.ack?"OK":"KO");
      }
    else
      {
	ROS_ERROR("Failed to call service kill_node_group");
      }

  }

  void kill_all_allocated_node_groups(void) {
    tbb::concurrent_hash_map<std::string,std::string,MyHashCompare>::iterator it,itn;
    for (it=allocated_node_group.begin();it!=allocated_node_group.end();++it) {
      kill_node_group(it->second,it->first);
    }
  }

}
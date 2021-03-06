/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Technische Universität Berlin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Piotr Gawlowicz <gawlowicz@tkn.tu-berlin.de>
 */


#ifndef MY_GYM_ENTITY_H
#define MY_GYM_ENTITY_H

#include "ns3/opengym-module.h"
#include "ns3/nstime.h"

#include "ns3/core-module.h"
#include "ns3/opengym-module.h"
#include "mygym.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/traffic-control-module.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <fstream>
#include <vector>

namespace ns3 {

class MyGymEnv : public OpenGymEnv
{
public:
  MyGymEnv ();
  MyGymEnv (Time stepTime);
  virtual ~MyGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  Ptr<OpenGymSpace> GetActionSpace();
  Ptr<OpenGymSpace> GetObservationSpace();
  bool GetGameOver();
  Ptr<OpenGymDataContainer> GetObservation();
  float GetReward();
  std::string GetExtraInfo();
  bool ExecuteActions(Ptr<OpenGymDataContainer> action);
  float minTh, maxTh;
  float bottleNeckLinkBw, bottleNeckLinkDelay, queueMaxSize;
  int onoffPackets, bulkPackets;
  bool SetInfo(float minTh, float maxTh, float bottleNeckLinkBw, float bottleNeckLinkDelay, float queueMaxSize);

  Ptr<QueueDisc> queue;
  static int queue_size;
  static int m_count;
  static double dataFromPPBP;
  static double dataFromBulk;
  static double totalDataFromPPBP;
  static double totalDataFromBulk;

  static long total_packet_size;
  static int packet_count;
  static long dequeue_packets;
  static vector<int> packetDelay;

  static double last_median_packet_delay;
  static double last_link_utilization;

  // the function has to be static to work with MakeBoundCallback
  // that is why we pass pointer to MyGymEnv instance to be able to store the context (node, etc)
  static void PerformCca(Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item);
  static void Enqueue(Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item);
  static void Dequeue(Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item);
  static void Drop(Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item);
  static void ReceivedFromBulkSender(Ptr<const Packet> p);
  static void ReceivedFromPPBPSender(Ptr<const Packet> p);

private:
  uint32_t nodeNum = 7;
  uint32_t actionNum = 1;
  float low = 0.0;
  float high = 1000.0;
  void ScheduleNextStateRead();
  Time m_interval;
};

}


#endif // MY_GYM_ENTITY_H

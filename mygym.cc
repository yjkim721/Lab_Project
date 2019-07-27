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

#include "mygym.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/node-list.h"
#include "ns3/log.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/traffic-control-module.h"

#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace ns3 {

double MyGymEnv::dataFromBulk = 0;
double MyGymEnv::dataFromPPBP = 0;
double MyGymEnv::totalDataFromBulk = 0;
double MyGymEnv::totalDataFromPPBP = 0;
int MyGymEnv::queue_size = 0;
int MyGymEnv::m_count = 0;

long MyGymEnv::total_packet_size = 0;
int MyGymEnv::packet_count = 0;
long MyGymEnv::dequeue_packets = 0;
vector<int> MyGymEnv::packetDelay;

double MyGymEnv::last_link_utilization = -1;
double MyGymEnv::last_median_packet_delay = -1;

NS_LOG_COMPONENT_DEFINE ("MyGymEnv");

NS_OBJECT_ENSURE_REGISTERED (MyGymEnv);

MyGymEnv::MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
  m_interval = Seconds(0.1);
  Simulator::Schedule (Seconds(0.0), &MyGymEnv::ScheduleNextStateRead, this);
}

MyGymEnv::MyGymEnv (Time stepTime)
{
  NS_LOG_FUNCTION (this);
  m_interval = stepTime;

  Simulator::Schedule (Seconds(0.0), &MyGymEnv::ScheduleNextStateRead, this);
}

void
MyGymEnv::ScheduleNextStateRead ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Schedule (m_interval, &MyGymEnv::ScheduleNextStateRead, this);
  Notify();
}

MyGymEnv::~MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
MyGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyGymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<MyGymEnv> ()
  ;
  return tid;
}

void
MyGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

/*
Define observation space
*/
Ptr<OpenGymSpace>
MyGymEnv::GetObservationSpace()
{
  std::vector<uint32_t> shape = {nodeNum,};
  std::string dtype = TypeNameGet<float> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("GetObservationSpace: " << space);
  return space;
}

/*
Define action space
*/
Ptr<OpenGymSpace>
MyGymEnv::GetActionSpace()
{
  NS_LOG_FUNCTION (this);
  std::vector<uint32_t> shape = {actionNum,};
  std::string dtype = TypeNameGet<float> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("GetActionSpace: " << space);
  return space;
}

/*
Define game over condition
*/
bool
MyGymEnv::GetGameOver()
{
  bool isGameOver = false;
  bool test = false;
  static float stepCounter = 0.0;
  stepCounter += 1;
  if (stepCounter == 10 && test) {
      isGameOver = true;
  }
  NS_LOG_UNCOND ("MyGetGameOver: " << isGameOver);
  return isGameOver;
}

/*
Collect observations
*/
Ptr<OpenGymDataContainer>
MyGymEnv::GetObservation()
{
  NS_LOG_FUNCTION (this);
  std::vector<uint32_t> shape = {nodeNum,};
  Ptr<OpenGymBoxContainer<float> > box = CreateObject<OpenGymBoxContainer<float> >(shape);

  box->AddValue(queueMaxSize);
  box->AddValue(queue_size);
  box->AddValue(m_count);
  box->AddValue(minTh);
  box->AddValue(maxTh);
  box->AddValue(bottleNeckLinkBw);
  box->AddValue(bottleNeckLinkDelay);
  NS_LOG_UNCOND ("MyGetObservation: " << box);
  return box;
}

/*
Define reward function
*/
float
MyGymEnv::GetReward()
{
  //static float reward = 1;
  int reward = 0;
  NS_LOG_UNCOND("reward" << reward);
  if (packet_count > 0) {
    int idx = packetDelay.size() / 2;
    NS_LOG_UNCOND("idx" << idx);
    int meanPktSize = total_packet_size / packet_count;
    NS_LOG_UNCOND("meanPktSize" << meanPktSize);
    double bandwidth = bottleNeckLinkBw * 1000000;
    NS_LOG_UNCOND("bandwidth" << bandwidth);
    double median_pkt_delay = packetDelay[idx] * 8 * meanPktSize / bandwidth;
    NS_LOG_UNCOND("median_pkt_delay" << median_pkt_delay);
    double outcoming_rate = dequeue_packets * 8 / 0.1;
    NS_LOG_UNCOND("outcoming_rate" << outcoming_rate);
    double link_utilization = outcoming_rate / bandwidth;
    NS_LOG_UNCOND("link_utilization" << link_utilization);

    if (last_link_utilization < 0 || last_median_packet_delay < 0) {
      reward = 0;
    }
    else if (link_utilization - last_link_utilization > 0.001 && last_median_packet_delay - median_pkt_delay > 0.001) {
      reward = 2;
    }
    else if (link_utilization - last_link_utilization > 0.001 || last_median_packet_delay - median_pkt_delay > 0.001){
      reward = 1;
    }
    else if (link_utilization - last_link_utilization < -0.001 && last_median_packet_delay - median_pkt_delay < -0.001){
      reward = -2;
    }
    else if (link_utilization - last_link_utilization < -0.001 || last_median_packet_delay - median_pkt_delay < -0.001){
      reward = -1;
    }
    else{
      reward = 0;
    }
    last_link_utilization = link_utilization;
    last_median_packet_delay = median_pkt_delay;
    NS_LOG_UNCOND ("MyGetReward: " << reward << "," << link_utilization << "," << median_pkt_delay);
  }

  packet_count = 0;
  packetDelay.clear();
  total_packet_size = 0;
  dequeue_packets = 0;
  return reward;
}

/*
Define extra info. Optional
*/
std::string
MyGymEnv::GetExtraInfo()
{
  std::string myInfo = std::to_string(dataFromPPBP*8/1000000) + ",";
  myInfo += std::to_string(dataFromBulk*8/1000000) + ",";
  myInfo += std::to_string(last_link_utilization) + ",";
  myInfo += std::to_string(last_median_packet_delay);
  //dataFromBulk = 0;
  //dataFromPPBP = 0;
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo << "/" << Simulator::Now().GetSeconds());
  return myInfo;
}

/*
Execute received actions
*/
bool
MyGymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
  Ptr<OpenGymBoxContainer<float> > box = DynamicCast<OpenGymBoxContainer<float> >(action);
  float alpha = box -> GetValue(0);
  this->maxTh = alpha;
  int new_maxTh = queueMaxSize * alpha;
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (new_maxTh));
  NS_LOG_UNCOND("myGymEnv(ExecuteActions): " << new_maxTh);
  //float beta = box -> GetValue(1);
  //int new_minTh = queueMaxSize * alpha;
  //this->minTh = new_minTh;
  //StaticCast<RedQueueDisc> (queue)->SetTh (alpha, beta);
  return true;
}

/*
Set minTh, maxTh, bottleNeckLinkBw. bottleNeckLinkDelay
*/
bool
MyGymEnv::SetInfo(float minTh, float maxTh, float bottleNeckLinkBw, float bottleNeckLinkDelay, float queueMaxSize){
  this->minTh = minTh;
  this->maxTh = maxTh;
  this->bottleNeckLinkBw = bottleNeckLinkBw;
  this->bottleNeckLinkDelay = bottleNeckLinkDelay;
  this->queueMaxSize = queueMaxSize;
  return true;
}

void
MyGymEnv::PerformCca (Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item)
{
  NS_LOG_UNCOND ("PerformCca: ");
}

void
MyGymEnv::Enqueue (Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item)
{
  queue_size += 1;
  packetDelay.push_back(queue_size);
  total_packet_size += item -> GetPacket() -> GetSize();
  packet_count += 1;
  m_count += 1;
  //NS_LOG_UNCOND ("Enqueue: " << queue_size);
}

void
MyGymEnv::Dequeue (Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item)
{
  queue_size -= 1;
  dequeue_packets += item -> GetPacket() -> GetSize();
  //NS_LOG_UNCOND ("Dequeue" << queue_size);
}

void
MyGymEnv::Drop (Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item)
{
  m_count = 0;
}

void
MyGymEnv::ReceivedFromBulkSender(Ptr<const Packet> p){
  dataFromBulk += p->GetSize();
  totalDataFromBulk += p->GetSize();
}

void
MyGymEnv::ReceivedFromPPBPSender(Ptr<const Packet> p){
  dataFromPPBP += p->GetSize();
  totalDataFromPPBP += p->GetSize();
}

} // ns3 namespace

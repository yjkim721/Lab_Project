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

NS_LOG_COMPONENT_DEFINE ("MyGymEnv");

NS_OBJECT_ENSURE_REGISTERED (MyGymEnv);

int MyGymEnv::queue_size = 0;
int MyGymEnv::m_count = 0;
double MyGymEnv::avgPacketDelay = 0;
double MyGymEnv::lastPacketDelay = 9999;
double MyGymEnv::totalPacketDelay = 0;
double MyGymEnv::dropRate = 0;
int MyGymEnv::dropPackets = 0;
int MyGymEnv::dequeuePackets = 0;
int MyGymEnv::receivedPackets = 0;
int MyGymEnv::packetNum = 0;
int MyGymEnv::totalPacketNum = 0;

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
  static float reward = 0.0;
  avgPacketDelay = std::isnan(avgPacketDelay / packetNum) ? 0 : avgPacketDelay / packetNum;
  dropRate = std::isnan((double)dropPackets/(packetNum + dropPackets)) ? 0 : (double)dropPackets/(packetNum + dropPackets);
  receivedPackets = packetNum + dropPackets;
  if(this-> minTh > this->maxTh){
    reward = -2;
  }else{
      if (this->minTh < avgPacketDelay && avgPacketDelay < this->maxTh) {
        reward = std::max(avgPacketDelay-lastPacketDelay, 0.0);
      }
      else if (avgPacketDelay >= this->maxTh){
        reward = -1;
      }
      else {
        reward = 0;
      }
  }
  lastPacketDelay = avgPacketDelay;
  avgPacketDelay = 0;
  packetNum = 0;
  dropPackets = 0;
  NS_LOG_UNCOND ("MyGetReward: " << reward);
  return reward;
}

/*
Define extra info. Optional
*/
std::string
MyGymEnv::GetExtraInfo()
{
  std::string myInfo = std::to_string(totalPacketDelay/totalPacketNum);
  myInfo += "," + std::to_string(lastPacketDelay);
  myInfo += "," + std::to_string(dropRate);
  myInfo += "," + std::to_string(dequeuePackets);
  myInfo += "," + std::to_string(receivedPackets);
  NS_LOG_UNCOND("MyGetExtraInfo(totalPacketDelay): " << myInfo);
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
  float beta = box -> GetValue(1);
  int new_minTh = queueMaxSize * alpha;
  int new_maxTh = queueMaxSize * beta;
  this->minTh = new_minTh;
  this->maxTh = new_maxTh;
  NS_LOG_UNCOND("myGymEnv(ExecuteActions): " << new_minTh << "," << new_maxTh);
  //Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (new_minTh));
  //Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (new_maxTh));
  //StaticCast<RedQueueDisc> (queue)->SetTh (alpha, beta);
  return true;
}

/*
Set minTh, maxTh, bottleNeckLinkBw. bottleNeckLinkDelay
*/
bool
MyGymEnv::SetInfo(int minTh, int maxTh, int maxSizeValue, float bottleNeckLinkBw, float bottleNeckLinkDelay){
  this->minTh = minTh;
  this->maxTh = maxTh;
  this->bottleNeckLinkBw = bottleNeckLinkBw;
  this->bottleNeckLinkDelay = bottleNeckLinkDelay;
  this->queueMaxSize = maxSizeValue;
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
  avgPacketDelay += queue_size;
  totalPacketDelay += queue_size;
  packetNum += 1;
  totalPacketNum += 1;
  queue_size += 1;
  m_count += 1;
  //NS_LOG_UNCOND ("Enqueue: " << queue_size);
}

void
MyGymEnv::Dequeue (Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item)
{
  queue_size -= 1;
  dequeuePackets += 1;
  //NS_LOG_UNCOND ("Dequeue" << queue_size);
}

void
MyGymEnv::Drop (Ptr<MyGymEnv> entity, double duration, Ptr< const QueueDiscItem > item)
{
  m_count = 0;
  dropPackets += 1;
  NS_LOG_UNCOND ("!!!!!!!!!!!!!!!!!!!!!!!!!DROP" << queue_size << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
}

} // ns3 namespace

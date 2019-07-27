/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

/* This script configures two nodes connected vis CSMA channel. One of the
*  nodes transmits packets via PPBP application to the other node.
*  Command line parameters are the simulationTime and verbose for logging.
*  Example run: Copy to scratch folder and run
*  ./waf --run "scratch/PPBP-application-test --simulationTime=10.0 --verbose=true"
*  Author: Sharan Naribole <nsharan@rice.edu>
*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-helper.h"

#include "ns3/opengym-module.h"
#include "mygym.h"

#include <vector>

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TEST_SIMPLE_PPBP");

// The times
double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;
double calculate_start_time;
double calculate_stop_time;

NodeContainer n0n5, n1n5, n2n5, n3n5, n4n5, n5n6, n6n7;
Ipv4InterfaceContainer i0i5, i1i5, i2i5, i3i5, i4i5, i5i6, i6i7;

long total_packet_size = 0;
long data_from_ppbp = 0;
long data_from_bulk = 0;
int packet_count = 0;
int queue_size = 0;
long dequeue_packets = 0;
vector<int> packetDelay;

void Enqueue(Ptr< const QueueDiscItem > item){
  //NS_LOG_UNCOND("Enqueue");
  queue_size += 1;
  packetDelay.push_back(queue_size);
  total_packet_size += item -> GetPacket() -> GetSize();
  packet_count += 1;
}

void Dequeue(Ptr< const QueueDiscItem > item){
  //NS_LOG_UNCOND("Dequeue");
  queue_size -= 1;
  dequeue_packets += item -> GetPacket() -> GetSize();
}

void Drop(Ptr< const QueueDiscItem > item){
  //NS_LOG_UNCOND("Enqueue");
}

void TxOfBulk(Ptr<const Packet> p){
  //NS_LOG_UNCOND("Tx of Bulk");
  data_from_bulk += p -> GetSize();
}

void TxOfPPBP(Ptr<const Packet> p){
  data_from_ppbp += p -> GetSize();
}

void initializeCalculation(){
  cout << "initialize Calculation" << endl;
  queue_size = 0;
  dequeue_packets = 0;
  data_from_bulk = 0;
  data_from_ppbp = 0;
  packetDelay.clear();
}

void getCalculation(double bandwidth){
  cout << "Get Calculation" << endl;
  int idx = packetDelay.size() / 2;
  int meanPktSize = total_packet_size / packet_count;
  double median_pkt_delay = packetDelay[idx] * 8 * meanPktSize / bandwidth;
  double outcoming_rate = dequeue_packets * 8 / (calculate_stop_time - calculate_start_time);
  double link_utilization = outcoming_rate / bandwidth;
  cout << "Median Pkt Delay: " << median_pkt_delay << " sec" << endl;
  cout << "Link Utilization: " << link_utilization << endl;
  cout << "PPBP Throughput: " << data_from_ppbp * 8 / 1000000 / (calculate_stop_time - calculate_start_time) << " Mbps" << endl;
  cout << "Bulk Throughput: " << data_from_bulk * 8 / 1000000 / (calculate_stop_time - calculate_start_time) << " Mbps" << endl;
}

void BuildApps() {
  ApplicationContainer clientApps1, clientApps2, clientApps3, clientApps4, clientApps5, sinkApp1;
  uint16_t port1 = 50001, port2 = 50002, port3 = 50003, port4 = 50004, port5 = 50005;
  uint32_t maxBytes = 0;

  PPBPHelper clientHelper1 = PPBPHelper ("ns3::UdpSocketFactory",
                     InetSocketAddress (i6i7.GetAddress (1), port1));
  clientApps1.Add (clientHelper1.Install (n0n5.Get (0)));
  clientApps1.Start (Seconds (client_start_time));
  clientApps1.Stop (Seconds (client_stop_time));

  BulkSendHelper clientHelper2 ("ns3::TcpSocketFactory", Address());
  clientHelper2.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  AddressValue remoteAddress2(InetSocketAddress (i6i7.GetAddress (1), port2));
  clientHelper2.SetAttribute ("Remote", remoteAddress2);
  clientApps2.Add(clientHelper2.Install (n1n5.Get (0)));
  clientApps2.Start (Seconds (client_start_time));
  clientApps2.Stop (Seconds (client_stop_time));

  BulkSendHelper clientHelper3 ("ns3::TcpSocketFactory", Address());
  clientHelper3.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  AddressValue remoteAddress3(InetSocketAddress (i6i7.GetAddress (1), port3));
  clientHelper3.SetAttribute ("Remote", remoteAddress3);
  clientApps3.Add(clientHelper3.Install (n2n5.Get (0)));
  clientApps3.Start (Seconds (client_start_time));
  clientApps3.Stop (Seconds (client_stop_time));

  BulkSendHelper clientHelper4 ("ns3::TcpSocketFactory", Address());
  clientHelper4.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  AddressValue remoteAddress4(InetSocketAddress (i6i7.GetAddress (1), port4));
  clientHelper4.SetAttribute ("Remote", remoteAddress4);
  clientApps4.Add(clientHelper4.Install (n3n5.Get (0)));
  clientApps4.Start (Seconds (client_start_time));
  clientApps4.Stop (Seconds (client_stop_time));

  BulkSendHelper clientHelper5 ("ns3::TcpSocketFactory", Address());
  clientHelper5.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  AddressValue remoteAddress5(InetSocketAddress (i6i7.GetAddress (1), port5));
  clientHelper5.SetAttribute ("Remote", remoteAddress5);
  clientApps5.Add(clientHelper5.Install (n4n5.Get (0)));
  clientApps5.Start (Seconds (client_start_time));
  clientApps5.Stop (Seconds (client_stop_time));

  // Sink App
  Address sinkLocalAddress1 (InetSocketAddress (Ipv4Address::GetAny (), port1));
  PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", sinkLocalAddress1);
  Address sinkLocalAddress2 (InetSocketAddress (Ipv4Address::GetAny (), port2));
  PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
  Address sinkLocalAddress3 (InetSocketAddress (Ipv4Address::GetAny (), port3));
  PacketSinkHelper sinkHelper3 ("ns3::TcpSocketFactory", sinkLocalAddress3);
  Address sinkLocalAddress4 (InetSocketAddress (Ipv4Address::GetAny (), port4));
  PacketSinkHelper sinkHelper4 ("ns3::TcpSocketFactory", sinkLocalAddress4);
  Address sinkLocalAddress5 (InetSocketAddress (Ipv4Address::GetAny (), port5));
  PacketSinkHelper sinkHelper5 ("ns3::TcpSocketFactory", sinkLocalAddress5);
  sinkApp1.Add(sinkHelper1.Install (n6n7.Get (1)));
  sinkApp1.Add(sinkHelper2.Install (n6n7.Get (1)));
  sinkApp1.Add(sinkHelper3.Install (n6n7.Get (1)));
  sinkApp1.Add(sinkHelper4.Install (n6n7.Get (1)));
  sinkApp1.Add(sinkHelper5.Install (n6n7.Get (1)));
  sinkApp1.Start (Seconds (sink_start_time));
  sinkApp1.Stop (Seconds (sink_stop_time));
}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("TEST_SIMPLE_PPBP", LOG_WARN);
  cout << "START Simulation" << endl;

  // Parameters of the RED
  double redLinkDataRateValue = 10.0;
  std::string redLinkDataRate = std::to_string(redLinkDataRateValue) + "Mbps";
  double redLinkDelayValue = 10.0;
  std::string redLinkDelay = std::to_string(redLinkDelayValue) + "ms";
  std::string accessLinkDataRate = "100Mbps";
  int accessLinkDelayValue = 1;
  std::string accessLinkDelay = "1ms";
  int maxSize = 999999;
  std::string qType = "ARED";

  // Parameters of the PPBP
  double meanBurstArrivals = 20;
  double meanBurstTimeLength = 0.02;
  double burstIntensityValue = 2;
  std::string burstIntensity;

  // Parameters of the scenario
  uint32_t simSeed = 1;
  double simulationTime = 100; //seconds
  double envStepTime = 0.1; //seconds, ns3gym env step time interval
  uint32_t openGymPort = 5555;
  uint32_t testArg = 0;

  // Parameters of simulation time
  global_start_time = 0.0;
  global_stop_time = simulationTime + 1.0;
  sink_start_time = global_start_time;
  sink_stop_time = global_stop_time + 0.1;
  client_start_time = sink_start_time + 0.2;
  client_stop_time = global_stop_time - 0.2;
  calculate_start_time = global_start_time + 10.0;
  calculate_stop_time = global_stop_time - 5.1;

  // Will only save in the directory if enable opts below
  CommandLine cmd;

  // required parameters for OpenGym interface
  cmd.AddValue ("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
  cmd.AddValue ("simSeed", "Seed for random generator. Default: 1", simSeed);
  // optional parameters
  cmd.AddValue ("simTime", "Simulation time in seconds. Default: 10s", simulationTime);
  cmd.AddValue ("stepTime", "Gym Env step time in seconds. Default: 0.1s", envStepTime);
  cmd.AddValue ("testArg", "Extra simulation argument. Default: 0", testArg);

  // topology parameters
  cmd.AddValue ("qType", "BottleNeck Queue Type. Default: ARED", qType);
  cmd.AddValue ("meanBurstArrivals", "average burst Arrivals(packet/sec)", meanBurstArrivals);
  cmd.AddValue ("meanBurstTimeLength", "scale parameter for Pareto", meanBurstTimeLength);
  cmd.AddValue ("burstIntensityValue", "burst intensity", burstIntensityValue);
  cmd.AddValue ("redLinkDataRateValue", "red link data rate value", redLinkDataRateValue);
  cmd.AddValue ("accessLinkDelayValue", "access link delay", accessLinkDelayValue);

  cmd.Parse (argc, argv);
  burstIntensity = std::to_string(burstIntensityValue) + "Mbps";
  redLinkDataRate = std::to_string(redLinkDataRateValue) + "Mbps";
  accessLinkDelay = std::to_string(accessLinkDelayValue) + "ms";

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (simSeed);

  // OpenGym Env
  Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
  Ptr<MyGymEnv> myGymEnv = CreateObject<MyGymEnv> (Seconds(envStepTime));
  myGymEnv->SetOpenGymInterface(openGymInterface);
  myGymEnv->SetInfo(0.0, 0.0, redLinkDataRateValue, redLinkDelayValue, (float)maxSize);

  // Creating Nodes
  NS_LOG_INFO ("Create nodes");
  NodeContainer c;
  c.Create (8);
  Names::Add ( "N0", c.Get (0));
  Names::Add ( "N1", c.Get (1));
  Names::Add ( "N2", c.Get (2));
  Names::Add ( "N3", c.Get (3));
  Names::Add ( "N4", c.Get (4));
  Names::Add ( "N5", c.Get (5));
  Names::Add ( "N6", c.Get (6));
  Names::Add ( "N7", c.Get (7));

  n0n5 = NodeContainer (c.Get (0), c.Get (5));
  n1n5 = NodeContainer (c.Get (1), c.Get (5));
  n2n5 = NodeContainer (c.Get (2), c.Get (5));
  n3n5 = NodeContainer (c.Get (3), c.Get (5));
  n4n5 = NodeContainer (c.Get (4), c.Get (5));
  n5n6 = NodeContainer (c.Get (5), c.Get (6));
  n6n7 = NodeContainer (c.Get (6), c.Get (7));

  // Configuration for Tcp
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  //Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000 - 42));
  //Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  //GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));

  // Configuratino for PPBP params
  cout << meanBurstArrivals << "," << meanBurstTimeLength << "," << burstIntensity << endl;
  Config::SetDefault ("ns3::PPBPApplication::MeanBurstArrivals", DoubleValue(meanBurstArrivals));
  Config::SetDefault ("ns3::PPBPApplication::MeanBurstTimeLength", DoubleValue(meanBurstTimeLength));
  Config::SetDefault ("ns3::PPBPApplication::BurstIntensity", DataRateValue ( DataRate (burstIntensity)));

  // Configuration for RED params
  NS_LOG_INFO ("Set RED params");
  Config::SetDefault ("ns3::RedQueueDisc::MaxSize", StringValue (std::to_string(maxSize)+"p"));
  Config::SetDefault ("ns3::RedQueueDisc::Wait", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::ARED", BooleanValue (true));

  NS_LOG_INFO ("Install internet stack on all nodes.");
  InternetStackHelper internet;
  internet.Install (c);

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue (std::to_string(maxSize)+"p"));

  TrafficControlHelper tchCodel;
  tchCodel.SetRootQueueDisc ("ns3::CoDelQueueDisc");
  Config::SetDefault ("ns3::CoDelQueueDisc::MaxSize", StringValue (std::to_string(maxSize)+"p"));

  TrafficControlHelper tchRed;
  tchRed.SetRootQueueDisc ("ns3::RedQueueDisc", "LinkBandwidth", StringValue (redLinkDataRate),
                            "LinkDelay", StringValue (redLinkDelay));

  NS_LOG_INFO ("Create channels");
  PointToPointHelper accessLinkn5n6;
  accessLinkn5n6.SetDeviceAttribute ("DataRate", StringValue (redLinkDataRate));
  accessLinkn5n6.SetChannelAttribute ("Delay", StringValue (redLinkDelay));
  NetDeviceContainer devn5n6 = accessLinkn5n6.Install (n5n6);

  QueueDiscContainer queueDiscs;
  if (qType.compare("ARED") == 0) {
    cout << "Install RED Queue" << endl;
    queueDiscs = tchRed.Install (devn5n6);
  }
  else {
    cout << "Install Codel Queue" << endl;
    queueDiscs = tchCodel.Install (devn5n6);
  }

  PointToPointHelper accessLinkn0n5;
  accessLinkn0n5.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
  accessLinkn0n5.SetChannelAttribute ("Delay", StringValue (accessLinkDelay));
  NetDeviceContainer devn0n5 = accessLinkn0n5.Install (n0n5);
  tchPfifo.Install (devn0n5);

  PointToPointHelper accessLinkn1n5;
  accessLinkn1n5.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
  accessLinkn1n5.SetChannelAttribute ("Delay", StringValue (accessLinkDelay));
  NetDeviceContainer devn1n5 = accessLinkn1n5.Install (n1n5);
  tchPfifo.Install (devn1n5);

  PointToPointHelper accessLinkn2n5;
  accessLinkn2n5.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
  accessLinkn2n5.SetChannelAttribute ("Delay", StringValue (accessLinkDelay));
  NetDeviceContainer devn2n5 = accessLinkn2n5.Install (n2n5);
  tchPfifo.Install (devn2n5);

  PointToPointHelper accessLinkn3n5;
  accessLinkn3n5.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
  accessLinkn3n5.SetChannelAttribute ("Delay", StringValue (accessLinkDelay));
  NetDeviceContainer devn3n5 = accessLinkn3n5.Install (n3n5);
  tchPfifo.Install (devn3n5);

  PointToPointHelper accessLinkn4n5;
  accessLinkn4n5.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
  accessLinkn4n5.SetChannelAttribute ("Delay", StringValue (accessLinkDelay));
  NetDeviceContainer devn4n5 = accessLinkn4n5.Install (n4n5);
  tchPfifo.Install (devn4n5);

  PointToPointHelper accessLinkn6n7;
  accessLinkn6n7.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
  accessLinkn6n7.SetChannelAttribute ("Delay", StringValue (accessLinkDelay));
  NetDeviceContainer devn6n7 = accessLinkn6n7.Install (n6n7);
  tchPfifo.Install (devn6n7);

  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  i0i5 = ipv4.Assign (devn0n5);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  i1i5 = ipv4.Assign (devn1n5);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  i2i5 = ipv4.Assign (devn2n5);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  i3i5 = ipv4.Assign (devn3n5);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  i4i5 = ipv4.Assign (devn4n5);

  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  i5i6 = ipv4.Assign (devn5n6);

  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  i6i7 = ipv4.Assign (devn6n7);

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  BuildApps ();
  Simulator::Stop (Seconds (global_stop_time));

  // Trace QueueDisc
  Ptr<QueueDisc> queue = queueDiscs.Get (0);
  myGymEnv->queue = queue;
  queue -> TraceConnectWithoutContext("Enqueue", MakeBoundCallback (&MyGymEnv::Enqueue, myGymEnv, 0.0f));
  queue -> TraceConnectWithoutContext("Dequeue", MakeBoundCallback (&MyGymEnv::Dequeue, myGymEnv, 0.0f));
  queue -> TraceConnectWithoutContext("Drop", MakeBoundCallback (&MyGymEnv::Drop, myGymEnv, 0.0f));

  // Trace Applications
  std::ostringstream oss1;
  oss1<< "/NodeList/0/ApplicationList/0/$ns3::PPBPApplication/Tx";
  Config::ConnectWithoutContext(oss1.str(), MakeCallback (&MyGymEnv::ReceivedFromPPBPSender));
  std::ostringstream oss2;
  oss2<< "/NodeList/*/ApplicationList/*/$ns3::BulkSendApplication/Tx";
  Config::ConnectWithoutContext(oss2.str(), MakeCallback (&MyGymEnv::ReceivedFromBulkSender));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
 }

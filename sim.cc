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
  *
  * Authors: Marcos Talau <talau@users.sourceforge.net>
  *          Duy Nguyen <duy@soe.ucsc.edu>
  * Modified by:   Pasquale Imputato <p.imputato@gmail.com>
  *
  */

 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/flow-monitor-helper.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/traffic-control-module.h"
 #include "ns3/packet-sink.h"

 #include "ns3/opengym-module.h"
 #include "mygym.h"

 using namespace ns3;

 NS_LOG_COMPONENT_DEFINE ("RedTests");

 uint32_t checkTimes;
 double avgQueueSize;

 // The times
 double global_start_time;
 double global_stop_time;
 double sink_start_time;
 double sink_stop_time;
 double client_start_time;
 double client_stop_time;

 NodeContainer n0n2;
 NodeContainer n1n2;
 NodeContainer n2n3;
 NodeContainer n3n4;
 NodeContainer n3n5;

 Ipv4InterfaceContainer i0i2;
 Ipv4InterfaceContainer i1i2;
 Ipv4InterfaceContainer i2i3;
 Ipv4InterfaceContainer i3i4;
 Ipv4InterfaceContainer i3i5;

 std::stringstream filePlotQueue;
 std::stringstream filePlotQueueAvg;

 void
 CheckQueueSize (Ptr<QueueDisc> queue)
 {
   uint32_t qSize = queue->GetCurrentSize ().GetValue ();

   avgQueueSize += qSize;
   checkTimes++;

   // check queue size every 1/100 of a second
   Simulator::Schedule (Seconds (0.01), &CheckQueueSize, queue);

   std::ofstream fPlotQueue (filePlotQueue.str ().c_str (), std::ios::out|std::ios::app);
   fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
   fPlotQueue.close ();

   std::ofstream fPlotQueueAvg (filePlotQueueAvg.str ().c_str (), std::ios::out|std::ios::app);
   fPlotQueueAvg << Simulator::Now ().GetSeconds () << " " << avgQueueSize / checkTimes << std::endl;
   fPlotQueueAvg.close ();
 }

 void
 BuildAppsTest (uint32_t test)
 {
   if ( (test == 1) || (test == 3) )
     {
       // SINK is in the right side
       uint16_t port = 5001;
       Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
       PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
       ApplicationContainer sinkApp = sinkHelper.Install (n3n4.Get (1));
       sinkApp.Start (Seconds (sink_start_time));
       sinkApp.Stop (Seconds (sink_stop_time));

       // Connection one
       // Clients are in left side
       /*
        * Create the OnOff applications to send TCP to the server
        * onoffhelper is a client that send data to TCP destination
        */
       OnOffHelper clientHelper1 ("ns3::TcpSocketFactory", Address ());
       clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
       clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
       clientHelper1.SetAttribute
         ("DataRate", DataRateValue (DataRate ("10Mb/s")));
       clientHelper1.SetAttribute
         ("PacketSize", UintegerValue (1000));

       ApplicationContainer clientApps1;
       AddressValue remoteAddress
         (InetSocketAddress (i3i4.GetAddress (1), port));
       clientHelper1.SetAttribute ("Remote", remoteAddress);
       clientApps1.Add (clientHelper1.Install (n0n2.Get (0)));
       clientApps1.Start (Seconds (client_start_time));
       clientApps1.Stop (Seconds (client_stop_time));

       // Connection two
       uint32_t maxBytes = 0;
       BulkSendHelper clientHelper2 ("ns3::TcpSocketFactory", InetSocketAddress (i3i4.GetAddress (1), port));
       // Set the amount of data to send in bytes.  Zero is unlimited.
       clientHelper2.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
       ApplicationContainer clientApp2 = clientHelper2.Install (n1n2.Get (0));
       clientApp2.Start (Seconds (3.0));
       clientApp2.Stop (Seconds (client_stop_time));

     }
   else // 4 or 5
     {
       // SINKs
       // #1
       uint16_t port1 = 50001;
       Address sinkLocalAddress1 (InetSocketAddress (Ipv4Address::GetAny (), port1));
       PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", sinkLocalAddress1);
       ApplicationContainer sinkApp1 = sinkHelper1.Install (n3n4.Get (1));
       sinkApp1.Start (Seconds (sink_start_time));
       sinkApp1.Stop (Seconds (sink_stop_time));
       // #2
       uint16_t port2 = 50002;
       Address sinkLocalAddress2 (InetSocketAddress (Ipv4Address::GetAny (), port2));
       PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
       ApplicationContainer sinkApp2 = sinkHelper2.Install (n3n5.Get (1));
       sinkApp2.Start (Seconds (sink_start_time));
       sinkApp2.Stop (Seconds (sink_stop_time));
       // #3
       uint16_t port3 = 50003;
       Address sinkLocalAddress3 (InetSocketAddress (Ipv4Address::GetAny (), port3));
       PacketSinkHelper sinkHelper3 ("ns3::TcpSocketFactory", sinkLocalAddress3);
       ApplicationContainer sinkApp3 = sinkHelper3.Install (n0n2.Get (0));
       sinkApp3.Start (Seconds (sink_start_time));
       sinkApp3.Stop (Seconds (sink_stop_time));
       // #4
       uint16_t port4 = 50004;
       Address sinkLocalAddress4 (InetSocketAddress (Ipv4Address::GetAny (), port4));
       PacketSinkHelper sinkHelper4 ("ns3::TcpSocketFactory", sinkLocalAddress4);
       ApplicationContainer sinkApp4 = sinkHelper4.Install (n1n2.Get (0));
       sinkApp4.Start (Seconds (sink_start_time));
       sinkApp4.Stop (Seconds (sink_stop_time));

       // Connection #1
       /*
        * Create the OnOff applications to send TCP to the server
        * onoffhelper is a client that send data to TCP destination
        */
       OnOffHelper clientHelper1 ("ns3::TcpSocketFactory", Address ());
       clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
       clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
       clientHelper1.SetAttribute
         ("DataRate", DataRateValue (DataRate ("10Mb/s")));
       clientHelper1.SetAttribute
         ("PacketSize", UintegerValue (1000));

       ApplicationContainer clientApps1;
       AddressValue remoteAddress1
         (InetSocketAddress (i3i4.GetAddress (1), port1));
       clientHelper1.SetAttribute ("Remote", remoteAddress1);
       clientApps1.Add (clientHelper1.Install (n0n2.Get (0)));
       clientApps1.Start (Seconds (client_start_time));
       clientApps1.Stop (Seconds (client_stop_time));

       // Connection #2
       OnOffHelper clientHelper2 ("ns3::TcpSocketFactory", Address ());
       clientHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
       clientHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
       clientHelper2.SetAttribute
         ("DataRate", DataRateValue (DataRate ("10Mb/s")));
       clientHelper2.SetAttribute
         ("PacketSize", UintegerValue (1000));

       ApplicationContainer clientApps2;
       AddressValue remoteAddress2
         (InetSocketAddress (i3i5.GetAddress (1), port2));
       clientHelper2.SetAttribute ("Remote", remoteAddress2);
       clientApps2.Add (clientHelper2.Install (n1n2.Get (0)));
       clientApps2.Start (Seconds (2.0));
       clientApps2.Stop (Seconds (client_stop_time));

       // Connection #3
       OnOffHelper clientHelper3 ("ns3::TcpSocketFactory", Address ());
       clientHelper3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
       clientHelper3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
       clientHelper3.SetAttribute
         ("DataRate", DataRateValue (DataRate ("10Mb/s")));
       clientHelper3.SetAttribute
         ("PacketSize", UintegerValue (1000));

       ApplicationContainer clientApps3;
       AddressValue remoteAddress3
         (InetSocketAddress (i0i2.GetAddress (0), port3));
       clientHelper3.SetAttribute ("Remote", remoteAddress3);
       clientApps3.Add (clientHelper3.Install (n3n4.Get (1)));
       clientApps3.Start (Seconds (3.5));
       clientApps3.Stop (Seconds (client_stop_time));

       // Connection #4
       OnOffHelper clientHelper4 ("ns3::TcpSocketFactory", Address ());
       clientHelper4.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
       clientHelper4.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
       clientHelper4.SetAttribute
         ("DataRate", DataRateValue (DataRate ("40b/s")));
       clientHelper4.SetAttribute
         ("PacketSize", UintegerValue (5 * 8)); // telnet

       ApplicationContainer clientApps4;
       AddressValue remoteAddress4
         (InetSocketAddress (i1i2.GetAddress (0), port4));
       clientHelper4.SetAttribute ("Remote", remoteAddress4);
       clientApps4.Add (clientHelper4.Install (n3n5.Get (1)));
       clientApps4.Start (Seconds (1.0));
       clientApps4.Stop (Seconds (client_stop_time));
     }
 }

 int
 main (int argc, char *argv[])
 {
   LogComponentEnable ("RedQueueDisc", LOG_LEVEL_INFO);

   uint32_t redTest;
   float redLinkDataRateF = 1.5;
   float redLinkDelayF = 20;
   std::string redLinkDataRate = "1.5Mbps";
   std::string redLinkDelay = "20ms";
   std::string accessLinkDataRate = "50Mbps";
   int maxSize = 400;
   int minTh = maxSize;
   int maxTh = maxSize;

   // Parameters of the scenario
   uint32_t simSeed = 1;
   double simulationTime = 50; //seconds
   double envStepTime = 0.1; //seconds, ns3gym env step time interval
   uint32_t openGymPort = 5555;
   uint32_t testArg = 0;

   std::string pathOut;
   bool writeForPlot = false;
   bool writePcap = false;
   bool flowMonitor = false;

   bool printRedStats = true;

   global_start_time = 0.0;
   global_stop_time = 50;
   sink_start_time = global_start_time;
   sink_stop_time = global_stop_time + 3.0;
   client_start_time = sink_start_time + 0.2;
   client_stop_time = global_stop_time - 2.0;

   // Configuration and command line parameter parsing
   redTest = 1;
   // Will only save in the directory if enable opts below
   pathOut = "."; // Current directory
   CommandLine cmd;
   // required parameters for OpenGym interface
   cmd.AddValue ("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
   cmd.AddValue ("simSeed", "Seed for random generator. Default: 1", simSeed);
   // optional parameters
   cmd.AddValue ("simTime", "Simulation time in seconds. Default: 10s", simulationTime);
   cmd.AddValue ("stepTime", "Gym Env step time in seconds. Default: 0.1s", envStepTime);
   cmd.AddValue ("testArg", "Extra simulation argument. Default: 0", testArg);

   cmd.AddValue ("testNumber", "Run test 1, 3, 4 or 5", redTest);
   cmd.AddValue ("pathOut", "Path to save results from --writeForPlot/--writePcap/--writeFlowMonitor", pathOut);
   cmd.AddValue ("writeForPlot", "<0/1> to write results for plot (gnuplot)", writeForPlot);
   cmd.AddValue ("writePcap", "<0/1> to write results in pcapfile", writePcap);
   cmd.AddValue ("writeFlowMonitor", "<0/1> to enable Flow Monitor and write their results", flowMonitor);

   cmd.Parse (argc, argv);
   if ( (redTest != 1) && (redTest != 3) && (redTest != 4) && (redTest != 5) )
     {
       NS_ABORT_MSG ("Invalid test number. Supported tests are 1, 3, 4 or 5");
     }

   RngSeedManager::SetSeed (1);
   RngSeedManager::SetRun (simSeed);

   // OpenGym Env
   Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
   Ptr<MyGymEnv> myGymEnv = CreateObject<MyGymEnv> (Seconds(envStepTime));
   myGymEnv->SetOpenGymInterface(openGymInterface);
   myGymEnv->SetInfo(minTh, maxTh, maxSize, redLinkDataRateF, redLinkDelayF);

   NS_LOG_INFO ("Create nodes");
   NodeContainer c;
   c.Create (6);
   Names::Add ( "N0", c.Get (0));
   Names::Add ( "N1", c.Get (1));
   Names::Add ( "N2", c.Get (2));
   Names::Add ( "N3", c.Get (3));
   Names::Add ( "N4", c.Get (4));
   Names::Add ( "N5", c.Get (5));
   n0n2 = NodeContainer (c.Get (0), c.Get (2));
   n1n2 = NodeContainer (c.Get (1), c.Get (2));
   n2n3 = NodeContainer (c.Get (2), c.Get (3));
   n3n4 = NodeContainer (c.Get (3), c.Get (4));
   n3n5 = NodeContainer (c.Get (3), c.Get (5));

   Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
   // 42 = headers size
   Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000 - 42));
   Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
   GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));

   uint32_t meanPktSize = 500;

   // RED params
   NS_LOG_INFO ("Set RED params");
   Config::SetDefault ("ns3::RedQueueDisc::MaxSize", StringValue ("400p"));
   Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (meanPktSize));
   Config::SetDefault ("ns3::RedQueueDisc::Wait", BooleanValue (true));
   Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (true));
   Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (0.002));
   Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (minTh));
   Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (maxTh));

   if (redTest == 3) // test like 1, but with bad params
     {
       Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (10));
       Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (0.003));
     }
   else if (redTest == 5) // test 5, same of test 4, but in byte mode
     {
       Config::SetDefault ("ns3::RedQueueDisc::MaxSize",
                           QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, 1000 * meanPktSize)));
       Config::SetDefault ("ns3::RedQueueDisc::Ns1Compat", BooleanValue (true));
       Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (5 * meanPktSize));
       Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (15 * meanPktSize));
     }

   NS_LOG_INFO ("Install internet stack on all nodes.");
   InternetStackHelper internet;
   internet.Install (c);

   TrafficControlHelper tchPfifo;
   uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
   tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));

   TrafficControlHelper tchRed;
   tchRed.SetRootQueueDisc ("ns3::RedQueueDisc", "LinkBandwidth", StringValue (redLinkDataRate),
                            "LinkDelay", StringValue (redLinkDelay));

   NS_LOG_INFO ("Create channels");
   PointToPointHelper p2p;

   p2p.SetQueue ("ns3::DropTailQueue");
   p2p.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
   p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
   NetDeviceContainer devn0n2 = p2p.Install (n0n2);
   tchPfifo.Install (devn0n2);

   p2p.SetQueue ("ns3::DropTailQueue");
   p2p.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
   p2p.SetChannelAttribute ("Delay", StringValue ("3ms"));
   NetDeviceContainer devn1n2 = p2p.Install (n1n2);
   tchPfifo.Install (devn1n2);

   p2p.SetQueue ("ns3::DropTailQueue");
   p2p.SetDeviceAttribute ("DataRate", StringValue (redLinkDataRate));
   p2p.SetChannelAttribute ("Delay", StringValue (redLinkDelay));
   NetDeviceContainer devn2n3 = p2p.Install (n2n3);
   // only backbone link has RED queue disc
   QueueDiscContainer queueDiscs = tchRed.Install (devn2n3);

   p2p.SetQueue ("ns3::DropTailQueue");
   p2p.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
   p2p.SetChannelAttribute ("Delay", StringValue ("4ms"));
   NetDeviceContainer devn3n4 = p2p.Install (n3n4);
   tchPfifo.Install (devn3n4);

   p2p.SetQueue ("ns3::DropTailQueue");
   p2p.SetDeviceAttribute ("DataRate", StringValue (accessLinkDataRate));
   p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
   NetDeviceContainer devn3n5 = p2p.Install (n3n5);
   tchPfifo.Install (devn3n5);

   NS_LOG_INFO ("Assign IP Addresses");
   Ipv4AddressHelper ipv4;

   ipv4.SetBase ("10.1.1.0", "255.255.255.0");
   i0i2 = ipv4.Assign (devn0n2);

   ipv4.SetBase ("10.1.2.0", "255.255.255.0");
   i1i2 = ipv4.Assign (devn1n2);

   ipv4.SetBase ("10.1.3.0", "255.255.255.0");
   i2i3 = ipv4.Assign (devn2n3);

   ipv4.SetBase ("10.1.4.0", "255.255.255.0");
   i3i4 = ipv4.Assign (devn3n4);

   ipv4.SetBase ("10.1.5.0", "255.255.255.0");
   i3i5 = ipv4.Assign (devn3n5);

   // Set up the routing
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

   // Trace QueueDisc
   Ptr<QueueDisc> queue = queueDiscs.Get (0);
   myGymEnv->queue = queue;
   queue -> TraceConnectWithoutContext("Enqueue", MakeBoundCallback (&MyGymEnv::Enqueue, myGymEnv, 0.0f));
   queue -> TraceConnectWithoutContext("Dequeue", MakeBoundCallback (&MyGymEnv::Dequeue, myGymEnv, 0.0f));
   queue -> TraceConnectWithoutContext("Drop", MakeBoundCallback (&MyGymEnv::Drop, myGymEnv, 0.0f));

   if (redTest == 5)
     {
       // like in ns2 test, r2 -> r1, have a queue in packet mode
       Ptr<QueueDisc> queue = queueDiscs.Get (1);

       queue->SetMaxSize (QueueSize ("1000p"));
       StaticCast<RedQueueDisc> (queue)->SetTh (5, 15);
     }

   BuildAppsTest (redTest);

   if (writePcap)
     {
       PointToPointHelper ptp;
       std::stringstream stmp;
       stmp << pathOut << "/red";
       ptp.EnablePcapAll (stmp.str ().c_str ());
     }

   Ptr<FlowMonitor> flowmon;
   if (flowMonitor)
     {
       FlowMonitorHelper flowmonHelper;
       flowmon = flowmonHelper.InstallAll ();
     }

   if (writeForPlot)
     {
       filePlotQueue << pathOut << "/" << "red-queue.plotme";
       filePlotQueueAvg << pathOut << "/" << "red-queue_avg.plotme";

       remove (filePlotQueue.str ().c_str ());
       remove (filePlotQueueAvg.str ().c_str ());
       Ptr<QueueDisc> queue = queueDiscs.Get (0);
       Simulator::ScheduleNow (&CheckQueueSize, queue);
     }

   Simulator::Stop (Seconds (sink_stop_time));
   Simulator::Run ();

   if (flowMonitor)
     {
       std::stringstream stmp;
       stmp << pathOut << "/red.flowmon";

       flowmon->SerializeToXmlFile (stmp.str ().c_str (), false, false);
     }

   if (printRedStats)
     {
       QueueDisc::Stats st = queueDiscs.Get (0)->GetStats ();
       std::cout << "*** RED stats from Node 2 queue disc ***" << std::endl;
       std::cout << st << std::endl;

       st = queueDiscs.Get (1)->GetStats ();
       std::cout << "*** RED stats from Node 3 queue disc ***" << std::endl;
       std::cout << st << std::endl;
     }

   Simulator::Destroy ();

   return 0;
 }

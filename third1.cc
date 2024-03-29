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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (3);

  PointToPointHelper pointToPoint1, pointToPoint2;
  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("7Mbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer devices1, devices2;
  devices1 = pointToPoint1.Install (nodes.Get(0), nodes.Get(1));
  devices2 = pointToPoint2.Install (nodes.Get(1), nodes.Get(2));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address1, address2;
  address1.SetBase ("10.1.1.0", "255.255.255.0");
  address2.SetBase ("10.2.2.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces1 = address1.Assign (devices1);
  Ipv4InterfaceContainer interfaces2 = address2.Assign (devices2);

  UdpEchoServerHelper echoServer1(9), echoServer2(10);

  //echoServer1 at port number 9
  ApplicationContainer serverApps1 = echoServer1.Install (nodes.Get (1));
  serverApps1.Start (Seconds (1.0));
  serverApps1.Stop (Seconds (10.0));
  
  //echoServer2 at port number 10
  ApplicationContainer serverApps2 = echoServer2.Install (nodes.Get (1));
  serverApps2.Start (Seconds (1.0));
  serverApps2.Stop (Seconds (10.0));

  //at node 0
  UdpEchoClientHelper echoClient1 (interfaces1.GetAddress (1), 9);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps1 = echoClient1.Install (nodes.Get (0));
  clientApps1.Start (Seconds (2.0));
  clientApps1.Stop (Seconds (10.0));


  //at node 2
  UdpEchoClientHelper echoClient2 (interfaces2.GetAddress (0), 10);
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps2 = echoClient2.Install (nodes.Get (2));
  clientApps2.Start (Seconds (2.0));
  clientApps2.Stop (Seconds (10.0));
  
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor=flowmon.InstallAll();
  NS_LOG_INFO("run simulaion");
  
  Simulator::Stop(Seconds(11.0));
  Simulator::Run();
  
  monitor->CheckForLostPackets();
  
  Ptr<Ipv4FlowClassifier> Classifier=DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  std::map<FlowId,FlowMonitor::FlowStats>stats=monitor->GetFlowStats();
  for(std::map<FlowId,FlowMonitor::FlowStats>::const_iterator i=stats.begin(); i!=stats.end();++i)
  {
  Ipv4FlowClassifier::FiveTuple t=Classifier->FindFlow(i->first);
  std::cout<<"flow="<<i->first<<"source address="<<t.sourceAddress<<"destination address="<<t.destinationAddress<<"source port="<<t.sourcePort<<""<<"destination port"<<t.destinationPort<<"\n";
  
  std::cout<<"flow="<<i->first<<"("<<t.sourceAddress<<"->"<<t.destinationAddress<<")\n";
  
  std::cout<<"TxBytes: " <<i->second.txBytes<<"\n";
  std::cout<<"RxBytes: " <<i->second.rxBytes<<"\n";
  std::cout<<"TxPackets: " <<i->second.txPackets<<"\n";
  std::cout<<"RxBytes: " <<i->second.rxPackets<<"\n";
  std::cout<<"Total time taken for transmission" <<i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds()<<std::endl;
  std::cout<<"throughput" <<i->second.rxBytes*8.0/(i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())/1000/1000<<"Mbps\n";
  
  }
  Simulator::Destroy ();
  return 0;
}

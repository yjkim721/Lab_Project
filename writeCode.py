ipv4s = ["i0i5", "i1i5", "i2i5", "i3i5", "i4i5", "i5i6" ,"i6i7", "i6i8", "i6i9", "i6i10", "i6i11"]
nodes = ["n0n5", "n1n5", "n2n5", "n3n5", "n4n5", "n5n6" ,"n6n7", "n6n8", "n6n9", "n6n10", "n6n11"]
delay = [1,2,3,4,5,1,2,3,4,5]
for i in range(4):
    print("OnOffHelper clientHelper%d (\"ns3::TcpSocketFactory\", Address ());" % (i+1))
    print("clientHelper%d.SetAttribute (\"OnTime\", StringValue (\"ns3::ConstantRandomVariable[Constant=1]\"));" % (i+1))
    print("clientHelper%d.SetAttribute (\"OffTime\", StringValue (\"ns3::ConstantRandomVariable[Constant=0]\"));" % (i+1))
    print("clientHelper%d.SetAttribute(\"DataRate\", DataRateValue (DataRate (\"10Mb/s\")));" % (i+1))
    print("clientHelper%d.SetAttribute(\"PacketSize\", UintegerValue (%d));" % (i+1, 1000))
    print("")
    print("ApplicationContainer clientApps%d;" % (i+1))
    print("AddressValue remoteAddress%d(InetSocketAddress (%s.GetAddress (1), port%d));" % (i+1,ipv4s[6+i], i+1))
    print("clientHelper%d.SetAttribute (\"Remote\", remoteAddress%d);" % (i+1, i+1))
    print("clientApps%d.Add (clientHelper%d.Install (%s.Get (0)));" % (i+1, i+1, nodes[i]))
    print("clientApps%d.Start (Seconds (client_start_time));" % (i+1))
    print("clientApps%d.Stop (Seconds (client_stop_time));" % (i+1))
    print("")

/*
 * C# IIO command server example
 *
 * Copyright 2012 Analog Devices Inc.
 *
 * Michael Hennerich <michael.hennerich@analog.com>
 */

using System;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;

/*  TEST CODE */

public class test_iio_srv
{
	
    public static void Main(string[] args)
    {

	IIOCmdSrv srv  = new IIOCmdSrv();
	// Connect to IIO command server
	srv.Connect("10.44.2.243");

	/* Show IIO Server Version Infromation */ 		
	Console.WriteLine("Server Version: " + srv.Version());	

	/* List Available IIO Devices on the Server */	
	IIODevice [] iio_devices = srv.ListDevices();
	foreach (var device in iio_devices)	
		Console.WriteLine("iio_devices are:" + device);
		
	/* A single server may handle multiple devices/drivers */
	IIODevice ad9467 = new IIODevice(srv, "cf-ad9467");

	/* Read the name attribute */
    Console.WriteLine("Device Name: " + ad9467.Read("name"));

	/* Read the in_voltage_scale_available attribute */
   	Console.WriteLine("in_voltage_scale_available: " + ad9467.Read("in_voltage_scale_available"));

	/* Set test modes for AD9467 Channel 0*/	
	ad9467.Write("in_voltage0_test_mode", "checkerboard");
		
	/* Get For  Samples */
	byte[] buf = ad9467.Sample(10);
	for (var i = 0; i < buf.Length; i+=2)
		Console.Write(string.Format("0x{0:X}\n",BitConverter.ToInt16(buf, i)));
		
	/* Turn test modes off */		
	ad9467.Write("in_voltage0_test_mode", "off");

	/* Get 10 Samples */
	buf = ad9467.Sample(10);
	for (var i = 0; i < buf.Length; i+=2)
		Console.Write(string.Format("0x{0:X}\n",BitConverter.ToInt16(buf, i)));		
		
	/* Test Direct Register Access methods */	
    Console.WriteLine("Device ID (dec): " + ad9467.RegRead(0x1));		
	
	/* List all device attributes */
	string [] attributes = 	ad9467.ListAttributes();
	foreach (string s in attributes)	
		Console.WriteLine("Attributes are: " + s);		
		
	/* Disconnect Server */
	srv.Disconnect();
    }	
	
//    public static void Main(string[] args)
//    {
//
//	IIOCmdSrv srv  = new IIOCmdSrv();
//	// Connect to IIO command server
//	srv.Connect("10.44.2.220");
//
//	/* Show IIO Server Version Infromation */ 		
//	Console.WriteLine("Server Version: " + srv.Version());	
//
//	/* List Available IIO Devices on the Server */	
//	IIODevice [] iio_devices = srv.ListDevices();
//	foreach (var device in iio_devices)	
//		Console.WriteLine("iio_devices are:" + device);
//		
//	/* A single server may handle multiple devices/drivers */
//	IIODevice ad9643 = new IIODevice(srv, "cf-ad9643-core-lpc");
//	IIODevice ad9523 = new IIODevice(srv, "ad9523-lpc");
//	/* Read the name attribute */
//    Console.WriteLine("Device Name: " + ad9643.Read("name"));
//
//	/* Read the in_voltage_scale_available attribute */
//   	Console.WriteLine("in_voltage_scale_available: " + ad9643.Read("in_voltage_scale_available"));
//
//	/* Set test modes for AD9643 Channel 0 and 1 */	
//	ad9643.Write("in_voltage0_test_mode", "checkerboard");
//	ad9643.Write("in_voltage1_test_mode", "pos_fullscale");	
//		
//	/* Get 10 Samples */
//	byte[] buf = ad9643.Sample(10);
//	for (var i = 0; i < buf.Length; i+=2)
//		Console.Write(string.Format("0x{0,4:X}\n",BitConverter.ToInt16(buf, i)));
//
//		
//	ad9643.ScanElementWrite("in_voltage0_en", "0");	
//	buf = ad9643.Sample(10);
//
//		
//	ad9643.BufferSetLength(15);	
//	Console.WriteLine("BufferGetLenght: " + ad9643.BufferGetLength());
//
//    Console.WriteLine("BufferIsEnabled: " + ad9643.BufferIsEnabled());
//
//		
//	/* Turn test modes off */		
//	ad9643.Write("in_voltage0_test_mode", "off");
//	ad9643.Write("in_voltage1_test_mode", "off");
//	
//		
//	/* Test Direct Register Access methods */	
//    Console.WriteLine("Device ID: (dec)" + ad9643.RegRead(0x1));		
//	ad9643.RegWrite(0x20, 0xAB);			
//	Console.WriteLine("Reg 0x20: (dec)" + ad9643.RegRead(0x20));
//	
//	/* List all device attributes */
//	string [] attributes = 	ad9643.ListAttributes();
//	foreach (string s in attributes)	
//		Console.WriteLine("Attributes are:" + s);
//
//	/* List all device scan elements */	
//	string [] scan_elements = ad9523.ListScanElements();
//	if (scan_elements != null)
//	foreach (string s in scan_elements)	
//		Console.WriteLine("Scan_elements are:" + s);		
//		
//	/* Disconnect Server */
//	srv.Disconnect();
//    }
}
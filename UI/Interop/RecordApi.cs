﻿using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI
{
	class RecordApi
	{
		private const string DllPath = "MesenSCore.dll";

		[DllImport(DllPath)] public static extern void AviRecord([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filename, VideoCodec codec, UInt32 compressionLevel);
		[DllImport(DllPath)] public static extern void AviStop();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool AviIsRecording();

		[DllImport(DllPath)] public static extern void WaveRecord([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filename);
		[DllImport(DllPath)] public static extern void WaveStop();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool WaveIsRecording();

		[DllImport(DllPath)] public static extern void MoviePlay([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filename);
		[DllImport(DllPath)] public static extern void MovieRecord(ref RecordMovieOptions options);
		[DllImport(DllPath)] public static extern void MovieStop();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool MoviePlaying();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool MovieRecording();
	}

	public enum RecordMovieFrom
	{
		StartWithoutSaveData,
		StartWithSaveData,
		CurrentState
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public struct RecordMovieOptions
	{
		private const int AuthorMaxSize = 250;
		private const int DescriptionMaxSize = 10000;
		private const int FilenameMaxSize = 2000;

		public RecordMovieOptions(string filename, string author, string description, RecordMovieFrom recordFrom)
		{
			Author = Encoding.UTF8.GetBytes(author);
			Array.Resize(ref Author, AuthorMaxSize);
			Author[AuthorMaxSize - 1] = 0;

			Description = Encoding.UTF8.GetBytes(description.Replace("\r", ""));
			Array.Resize(ref Description, DescriptionMaxSize);
			Description[DescriptionMaxSize - 1] = 0;

			Filename = Encoding.UTF8.GetBytes(filename);
			Array.Resize(ref Filename, FilenameMaxSize);
			Filename[FilenameMaxSize - 1] = 0;

			RecordFrom = recordFrom;
		}

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = FilenameMaxSize)]
		public byte[] Filename;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = AuthorMaxSize)]
		public byte[] Author;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = DescriptionMaxSize)]
		public byte[] Description;

		public RecordMovieFrom RecordFrom;
	}
}

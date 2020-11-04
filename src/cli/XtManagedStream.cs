using System;
using System.Runtime.InteropServices;
using static Xt.XtNative;

namespace Xt
{
	sealed class XtManagedStream : IDisposable
	{
		static T[][] CreateNonInterleavedBuffer<T>(int channels, int elements)
		{
			T[][] result = new T[channels][];
			for (int i = 0; i < channels; i++)
				result[i] = new T[elements];
			return result;
		}

		static Array CreateNonInterleavedBuffer(XtSample sample, int channels, int frames)
		{
			switch (sample)
			{
				case XtSample.UInt8:
					return CreateNonInterleavedBuffer<byte>(channels, frames);
				case XtSample.Int16:
					return CreateNonInterleavedBuffer<short>(channels, frames);
				case XtSample.Int24:
					return CreateNonInterleavedBuffer<byte>(channels, frames * 3);
				case XtSample.Int32:
					return CreateNonInterleavedBuffer<int>(channels, frames);
				case XtSample.Float32:
					return CreateNonInterleavedBuffer<float>(channels, frames);
				default:
					throw new ArgumentException();
			}
		}

		private static Array CreateInterleavedBuffer(XtSample sample, int channels, int frames)
		{
			switch (sample)
			{
				case XtSample.UInt8:
					return new byte[channels * frames];
				case XtSample.Int16:
					return new short[channels * frames];
				case XtSample.Int24:
					return new byte[channels * 3 * frames];
				case XtSample.Int32:
					return new int[channels * frames];
				case XtSample.Float32:
					return new float[channels * frames];
				default:
					throw new ArgumentException();
			}
		}

		private static void CopyInterleavedBufferFromNative(XtSample sample, IntPtr raw, Array managed, int channels, int frames)
		{
			switch (sample)
			{
				case XtSample.UInt8:
					Marshal.Copy(raw, (byte[])managed, 0, frames * channels);
					break;
				case XtSample.Int16:
					Marshal.Copy(raw, (short[])managed, 0, frames * channels);
					break;
				case XtSample.Int24:
					Marshal.Copy(raw, (byte[])managed, 0, frames * channels * 3);
					break;
				case XtSample.Int32:
					Marshal.Copy(raw, (int[])managed, 0, frames * channels);
					break;
				case XtSample.Float32:
					Marshal.Copy(raw, (float[])managed, 0, frames * channels);
					break;
				default:
					throw new ArgumentException();
			}
		}

		private static void CopyInterleavedBufferToNative(XtSample sample, Array managed, IntPtr raw, int channels, int frames)
		{
			switch (sample)
			{
				case XtSample.UInt8:
					Marshal.Copy((byte[])managed, 0, raw, frames * channels);
					break;
				case XtSample.Int16:
					Marshal.Copy((short[])managed, 0, raw, frames * channels);
					break;
				case XtSample.Int24:
					Marshal.Copy((byte[])managed, 0, raw, frames * channels * 3);
					break;
				case XtSample.Int32:
					Marshal.Copy((int[])managed, 0, raw, frames * channels);
					break;
				case XtSample.Float32:
					Marshal.Copy((float[])managed, 0, raw, frames * channels);
					break;
				default:
					throw new ArgumentException();
			}
		}

		private static unsafe void CopyNonInterleavedBufferFromNative(XtSample sample, IntPtr raw, Array managed, int channels, int frames)
		{
			void** data = (void**)raw.ToPointer();
			switch (sample)
			{
				case XtSample.UInt8:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(new IntPtr(data[i]), ((byte[][])managed)[i], 0, frames);
					break;
				case XtSample.Int16:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(new IntPtr(data[i]), ((short[][])managed)[i], 0, frames);
					break;
				case XtSample.Int24:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(new IntPtr(data[i]), ((byte[][])managed)[i], 0, frames * 3);
					break;
				case XtSample.Int32:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(new IntPtr(data[i]), ((int[][])managed)[i], 0, frames);
					break;
				case XtSample.Float32:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(new IntPtr(data[i]), ((float[][])managed)[i], 0, frames);
					break;
				default:
					throw new ArgumentException();
			}
		}

		private static unsafe void CopyNonInterleavedBufferToNative(XtSample sample, Array managed, IntPtr raw, int channels, int frames)
		{
			void** data = (void**)raw.ToPointer();
			switch (sample)
			{
				case XtSample.UInt8:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(((byte[][])managed)[i], 0, new IntPtr(data[i]), frames);
					break;
				case XtSample.Int16:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(((short[][])managed)[i], 0, new IntPtr(data[i]), frames);
					break;
				case XtSample.Int24:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(((byte[][])managed)[i], 0, new IntPtr(data[i]), frames * 3);
					break;
				case XtSample.Int32:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(((int[][])managed)[i], 0, new IntPtr(data[i]), frames);
					break;
				case XtSample.Float32:
					for (int i = 0; i < channels; i++)
						Marshal.Copy(((float[][])managed)[i], 0, new IntPtr(data[i]), frames);
					break;
				default:
					throw new ArgumentException();
			}
		}

		private readonly bool raw;
		private readonly object user;
		private readonly bool interleaved;
		private readonly XtXRunCallback xRunCallback;
		private readonly XtStreamCallback streamCallback;
		internal readonly XRunCallback nativeXRunCallback;
		internal readonly StreamCallback nativeStreamCallback;

		private IntPtr s;
		private Array inputInterleaved;
		private Array outputInterleaved;
		private Array inputNonInterleaved;
		private Array outputNonInterleaved;

		internal XtStream(bool interleaved, bool raw, XtStreamCallback streamCallback, XtXRunCallback xRunCallback, object user)
		{
			this.raw = raw;
			this.user = user;
			this.interleaved = interleaved;
			this.xRunCallback = xRunCallback;
			this.streamCallback = streamCallback;
			this.nativeXRunCallback = XRunCallback;
			this.nativeStreamCallback = StreamCallback;
		}

		public void Stop() => XtNative.HandleError(XtNative.XtStreamStop(s));
		public void Start() => XtNative.HandleError(XtNative.XtStreamStart(s));

		public void Dispose()
		{
			if (s != IntPtr.Zero)
				XtNative.XtStreamDestroy(s);
			s = IntPtr.Zero;
		}

		public int GetFrames()
		{
			XtNative.HandleError(XtNative.XtStreamGetFrames(s, out var result));
			return result;
		}

		public XtLatency GetLatency()
		{
			XtNative.HandleError(XtNative.XtStreamGetLatency(s, out var result));
			return result;
		}

		public XtFormat GetFormat()
		{
			return Marshal.PtrToStructure<XtFormat>(XtNative.XtStreamGetFormat(s));
		}

		internal void Init(IntPtr s)
		{
			this.s = s;
			if (!raw)
			{
				int frames = GetFrames();
				XtFormat format = GetFormat();
				if (interleaved)
				{
					inputInterleaved = CreateInterleavedBuffer(format.mix.sample, format.channels.inputs, frames);
					outputInterleaved = CreateInterleavedBuffer(format.mix.sample, format.channels.outputs, frames);
				}
				else
				{
					inputNonInterleaved = CreateNonInterleavedBuffer(format.mix.sample, format.channels.inputs, frames);
					outputNonInterleaved = CreateNonInterleavedBuffer(format.mix.sample, format.channels.outputs, frames);
				}
			}
		}

		void XRunCallback(int index, IntPtr user) => xRunCallback(index, this.user);

		void StreamCallback(IntPtr stream, in XtBuffer buffer, in XtTime time, ulong error, IntPtr user)
		{
			XtFormat format = GetFormat();
			object inData = raw ? (object)input : input == IntPtr.Zero ? null : interleaved ? inputInterleaved : inputNonInterleaved;
			object outData = raw ? (object)output : output == IntPtr.Zero ? null : interleaved ? outputInterleaved : outputNonInterleaved;

			if (!raw && inData != null)
				if (interleaved)
					CopyInterleavedBufferFromNative(format.mix.sample, input, (Array)inData, format.channels.inputs, frames);
				else
					CopyNonInterleavedBufferFromNative(format.mix.sample, input, (Array)inData, format.channels.inputs, frames);

			streamCallback(this, inData, outData, frames, time, position, timeValid, error, user);

			if (!raw && outData != null)
				if (interleaved)
					CopyInterleavedBufferToNative(format.mix.sample, (Array)outData, output, format.channels.outputs, frames);
				else
					CopyNonInterleavedBufferToNative(format.mix.sample, (Array)outData, output, format.channels.outputs, frames);
		}
	}
}
using System;

namespace Xt
{
    public unsafe class OnAggregate : OnBuffer
    {
        double[] _aggregate;
        double _attenuate = 1.0;

        public OnAggregate(OnBufferParams @params) : base(@params) { }
        public void Init(int maxFrames) => _aggregate = new double[maxFrames];

        protected override void ProcessBuffer(XtStream stream, in XtBuffer buffer, XtSafeBuffer safe)
        {
            bool native = Params.Native;
            var format = stream.GetFormat();
            bool interleaved = Params.Interleaved;
            Array.Clear(_aggregate, 0, buffer.frames);
            for (int f = 0; f < buffer.frames; f++)
            {
                for (int c = 0; c < format.channels.inputs; c++)
                {
                    if (!native && !interleaved) ProcessNonInterleavedSafeInput(in format, f, c, safe.GetInput());
                    if (!native && interleaved) ProcessInterleavedSafeInput(in format, f, c, safe.GetInput());
                    if (native && !interleaved) ProcessNonInterleavedNativeInput(in format, f, c, buffer.input);
                    if (native && interleaved) ProcessInterleavedNativeInput(in format, f, c, buffer.input);
                }
                _attenuate = Math.Min(_attenuate, 1.0 / Math.Abs(_aggregate[f]));
                _aggregate[f] *= _attenuate;
                for (int c = 0; c < format.channels.outputs; c++)
                {
                    if (!native && !interleaved) ProcessNonInterleavedSafeOutput(in format, f, c, safe.GetOutput());
                    if (!native && interleaved) ProcessInterleavedSafeOutput(in format, f, c, safe.GetOutput());
                    if (native && !interleaved) ProcessNonInterleavedNativeOutput(in format, f, c, buffer.output);
                    if (native && interleaved) ProcessInterleavedNativeOutput(in format, f, c, buffer.output);
                }
            }
        }

        void ProcessNonInterleavedSafeInput(in XtFormat format, int f, int c, Array input)
        {
            switch (format.mix.sample)
            {
            case XtSample.Float32: _aggregate[f] += ((float[][])input)[c][f]; break;
            case XtSample.Int32: _aggregate[f] += (((int[][])input)[c][f]) / (double)int.MaxValue; break;
            case XtSample.Int16: _aggregate[f] += (((short[][])input)[c][f]) / (double)short.MaxValue; break;
            case XtSample.UInt8: _aggregate[f] += (((((byte[][])input)[c][f]) * 2.0) - 1.0) / byte.MaxValue; break;
            case XtSample.Int24:
            int value = (int)(((byte[][])input)[c][f * 3] << 8 | ((byte[][])input)[c][f * 3 + 1] << 16 | ((byte[][])input)[c][f * 3 + 2] << 24);
            _aggregate[f] += value / (double)int.MaxValue;
            break;
            default: throw new InvalidOperationException();
            }
        }

        void ProcessInterleavedSafeInput(in XtFormat format, int f, int c, Array input)
        {
            var pos = f * format.channels.inputs + c;
            switch (format.mix.sample)
            {
            case XtSample.Float32: _aggregate[f] += ((float[])input)[pos]; break;
            case XtSample.Int32: _aggregate[f] += (((int[])input)[pos]) / (double)int.MaxValue; break;
            case XtSample.Int16: _aggregate[f] += (((short[])input)[pos]) / (double)short.MaxValue; break;
            case XtSample.UInt8: _aggregate[f] += (((((byte[])input)[pos]) * 2.0) - 1.0) / byte.MaxValue; break;
            case XtSample.Int24:
            int value = ((byte[])input)[pos * 3] << 8 | ((byte[])input)[pos * 3 + 1] << 16 | ((byte[])input)[pos * 3 + 2] << 24;
            _aggregate[f] += value / (double)int.MaxValue;
            break;
            default: throw new InvalidOperationException();
            }
        }

        void ProcessNonInterleavedNativeInput(in XtFormat format, int f, int c, IntPtr input)
        {
            switch (format.mix.sample)
            {
            case XtSample.Float32: _aggregate[f] += ((float**)input)[c][f]; break;
            case XtSample.Int32: _aggregate[f] += (((int**)input)[c][f]) / (double)int.MaxValue; break;
            case XtSample.Int16: _aggregate[f] += (((short**)input)[c][f]) / (double)short.MaxValue; break;
            case XtSample.UInt8: _aggregate[f] += (((((byte**)input)[c][f]) * 2.0) - 1.0) / byte.MaxValue; break;
            case XtSample.Int24:
            int value = (((byte**)input)[c][f * 3] << 8 | ((byte**)input)[c][f * 3 + 1] << 16 | ((byte**)input)[c][f * 3 + 2] << 24);
            _aggregate[f] += value / (double)int.MaxValue;
            break;
            default: throw new InvalidOperationException();
            }
        }

        void ProcessInterleavedNativeInput(in XtFormat format, int f, int c, IntPtr input)
        {
            var pos = f * format.channels.inputs + c;
            switch (format.mix.sample)
            {
            case XtSample.Float32: _aggregate[f] += ((float*)input)[pos]; break;
            case XtSample.Int32: _aggregate[f] += (((int*)input)[pos]) / (double)int.MaxValue; break;
            case XtSample.Int16: _aggregate[f] += (((short*)input)[pos]) / (double)short.MaxValue; break;
            case XtSample.UInt8: _aggregate[f] += (((((byte*)input)[pos]) * 2.0) - 1.0) / byte.MaxValue; break;
            case XtSample.Int24:
            int value = (((byte*)input)[pos * 3] << 8 | ((byte*)input)[pos * 3 + 1] << 16 | ((byte*)input)[pos * 3 + 2] << 24);
            _aggregate[f] += value / (double)int.MaxValue;
            break;
            default: throw new InvalidOperationException();
            }
        }

        void ProcessNonInterleavedSafeOutput(in XtFormat format, int f, int c, Array output)
        {
            switch (format.mix.sample)
            {
            case XtSample.Float32: ((float[][])output)[c][f] = (float)_aggregate[f]; break;
            case XtSample.Int32: ((int[][])output)[c][f] = (int)(_aggregate[f] * int.MaxValue); break;
            case XtSample.Int16: ((short[][])output)[c][f] = (short)(_aggregate[f] * short.MaxValue); break;
            case XtSample.UInt8: ((byte[][])output)[c][f] = (byte)(((_aggregate[f] + 1.0) * 0.5) * byte.MaxValue); break;
            case XtSample.Int24:
            int value = (int)(_aggregate[f] * int.MaxValue);
            ((byte[][])output)[c][f * 3] = (byte)((value & 0x0000FF00) >> 8);
            ((byte[][])output)[c][f * 3 + 1] = (byte)((value & 0x00FF0000) >> 16);
            ((byte[][])output)[c][f * 3 + 2] = (byte)((value & 0xFF000000) >> 24);
            break;
            default: throw new InvalidOperationException();
            }
        }

        void ProcessInterleavedSafeOutput(in XtFormat format, int f, int c, Array output)
        {
            var pos = f * format.channels.outputs + c;
            switch (format.mix.sample)
            {
            case XtSample.Float32: ((float[])output)[pos] = (float)_aggregate[f]; break;
            case XtSample.Int32: ((int[])output)[pos] = (int)(_aggregate[f] * int.MaxValue); break;
            case XtSample.Int16: ((short[])output)[pos] = (short)(_aggregate[f] * short.MaxValue); break;
            case XtSample.UInt8: ((byte[])output)[pos] = (byte)(((_aggregate[f] + 1.0) * 0.5) * byte.MaxValue); break;
            case XtSample.Int24:
            int value = (int)(_aggregate[f] * (double)int.MaxValue);
            ((byte[])output)[pos * 3] = (byte)((value & 0x0000FF00) >> 8);
            ((byte[])output)[pos * 3 + 1] = (byte)((value & 0x00FF0000) >> 16);
            ((byte[])output)[pos * 3 + 2] = (byte)((value & 0xFF000000) >> 24);
            break;
            default: throw new InvalidOperationException();
            }
        }

        void ProcessNonInterleavedNativeOutput(in XtFormat format, int f, int c, IntPtr output)
        {
            switch (format.mix.sample)
            {
            case XtSample.Float32: ((float**)output)[c][f] = (float)_aggregate[f]; break;
            case XtSample.Int32: ((int**)output)[c][f] = (int)(_aggregate[f] * int.MaxValue); break;
            case XtSample.Int16: ((short**)output)[c][f] = (short)(_aggregate[f] * short.MaxValue); break;
            case XtSample.UInt8: ((byte**)output)[c][f] = (byte)(((_aggregate[f] + 1.0) * 0.5) * byte.MaxValue); break;
            case XtSample.Int24:
            int value = (int)(_aggregate[f] * int.MaxValue);
            ((byte**)output)[c][f * 3] = (byte)((value & 0x0000FF00) >> 8);
            ((byte**)output)[c][f * 3 + 1] = (byte)((value & 0x00FF0000) >> 16);
            ((byte**)output)[c][f * 3 + 2] = (byte)((value & 0xFF000000) >> 24);
            break;
            default: throw new InvalidOperationException();
            }
        }

        void ProcessInterleavedNativeOutput(in XtFormat format, int f, int c, IntPtr output)
        {
            var pos = f * format.channels.outputs + c;
            switch (format.mix.sample)
            {
            case XtSample.Float32: ((float*)output)[pos] = (float)_aggregate[f]; break;
            case XtSample.Int32: ((int*)output)[pos] = (int)(_aggregate[f] * int.MaxValue); break;
            case XtSample.Int16: ((short*)output)[pos] = (short)(_aggregate[f] * short.MaxValue); break;
            case XtSample.UInt8: ((byte*)output)[pos] = (byte)(((_aggregate[f] + 1.0) * 0.5) * byte.MaxValue); break;
            case XtSample.Int24:
            int value = (int)(_aggregate[f] * int.MaxValue);
            ((byte*)output)[pos * 3] = (byte)((value & 0x0000FF00) >> 8);
            ((byte*)output)[pos * 3 + 1] = (byte)((value & 0x00FF0000) >> 16);
            ((byte*)output)[pos * 3 + 2] = (byte)((value & 0xFF000000) >> 24);
            break;
            default: throw new InvalidOperationException();
            }
        }
    }
}
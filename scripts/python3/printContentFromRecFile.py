#!/usr/bin/env python3

import sys
import struct
import os
import csv
import cluonDataStructures_pb2
import opendlv_standard_message_set_v0_9_9_pb2


def try_parse_groundsteering(payload):
    try:
        msg = opendlv_standard_message_set_v0_9_9_pb2.opendlv_proxy_GroundSteeringReading()
        msg.ParseFromString(payload)
        if msg.HasField("groundSteering"):
            return msg.groundSteering
    except Exception as e:
        print(f"⚠️  Parse error: {e}", file=sys.stderr)
    return None


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input.rec> <output.csv>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    LENGTH_ENVELOPE_HEADER = 5
    buf = b""
    expectedBytes = 0
    consumedEnvelopeHeader = False
    ground_count = 0
    total_envelopes = 0

    output_dir = os.path.dirname(output_file)
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)

    with open(input_file, "rb") as f, open(output_file, "w", newline="") as out_csv:
        writer = csv.writer(out_csv, delimiter=';')
        writer.writerow(["Timestamp", "PredictedSteeringAngle", "ActualSteeringAngle"])

        while True:
            byte = f.read(1)
            if not byte:
                break
            buf += byte

            if consumedEnvelopeHeader and len(buf) >= expectedBytes:
                envelope = cluonDataStructures_pb2.cluon_data_Envelope()
                try:
                    envelope.ParseFromString(buf[:expectedBytes])
                    total_envelopes += 1

                    if envelope.dataType == 1090:  # GroundSteeringReading
                        timestamp_us = envelope.sent.seconds * 1_000_000 + envelope.sent.microseconds
                        steering = try_parse_groundsteering(envelope.serializedData)

                        if steering is not None:
                            if abs(steering) > 1.0:
                                print(f"⚠️  Unusual value: {steering:.2f} at {timestamp_us} µs")
                            print(f"→ {timestamp_us} µs | actualSteering: {steering:.6f}")
                            writer.writerow([timestamp_us, "", steering])
                            ground_count += 1

                except Exception as e:
                    print(f"⚠️  Envelope parse failed: {e}", file=sys.stderr)

                buf = buf[expectedBytes:]
                expectedBytes = 0
                consumedEnvelopeHeader = False

            if not consumedEnvelopeHeader and len(buf) >= LENGTH_ENVELOPE_HEADER:
                if buf[0] == 0x0D and buf[1] == 0xA4:
                    v = struct.unpack('<L', buf[1:5])
                    expectedBytes = v[0] >> 8
                    buf = buf[5:]
                    consumedEnvelopeHeader = True
                else:
                    buf = buf[1:]

    print(f"\n✔️ Done. Found {ground_count} GroundSteeringReading messages out of {total_envelopes} envelopes.")
    print(f"📄 Output saved to: {output_file}")


if __name__ == "__main__":
    main()


import serial
import pandas as pd
import os

data_list = []
output_file = 'data_output.xlsx'
file_exists = os.path.isfile(output_file)

ser = serial.Serial('COM10', 115200, timeout=1)
while True:
    print("Waiting for data")
    raw = ser.readline()
    if not raw:
        continue

    if b"Driver ready." in raw:
        print("Received Driver ready!")
        break

print("Sending READY signal to Arduino...")
ser.write(b"READY\n")

counter = 0
while True:
    if ser.in_waiting > 0:
        data = ser.readline().decode('utf-8').strip()
        
        if data.lower() == "stop":
            break

        print(f"{counter}: Recevied {data}")
        parts = data.split(',', 3)
        if len(parts) != 4:
            df = pd.DataFrame(data_list, columns=['Pozitie', 'Frecventa (Hz)', 'Masa (g)', 'Pattern vibratie'])

            if file_exists:
                with pd.ExcelWriter(output_file, mode='a', engine='openpyxl', if_sheet_exists='overlay') as writer:
                    df.to_excel(writer, index=False, header=False, startrow=writer.sheets['Sheet1'].max_row)
            else:
                df.to_excel(output_file, index=False)
            data_list.clear()
            print(f"Data written to {output_file}")
        else:
            counter += 1
            pos_s, freq_s, wt_s, pattern_s = parts
            try:
                position = int(pos_s.strip())
                frequency = float(freq_s.strip())
                weight    = float(wt_s.strip())
                # strip off the surrounding quotes, if any
                vibration_pattern = pattern_s.strip().strip('"')

                data_list.append([position, frequency, weight, vibration_pattern])
            except ValueError as e:
                print(f"Error parsing data: {data}")
                break



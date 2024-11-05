import csv
import matplotlib.pyplot as plt

class LapTimerTester:
    def __init__(self, file_path):
        self.file_path = file_path
        self.laptime_counter = 0
        self.absolute_laptimes = []
        self.all_rssi_values = []

    def read_csv_file(self):
        """Reads the CSV file and processes the data for lap timing."""
        ADC_threshold = 330  # Threshold for lap detection
        over_threshold = False
        debounce_threshold = 5000
        threshold_counter = 0

        try:
            with open(self.file_path, 'r') as file:
                reader = csv.reader(file)
                next(reader, None)  

                for row in reader:
                    rssi_value = int(row[0])
                    self.all_rssi_values.append(rssi_value)
                    self.laptime_counter += 1

                    # Detect when the RSSI value crosses the threshold
                    if rssi_value >= ADC_threshold and not over_threshold and threshold_counter >= debounce_threshold:
                        over_threshold = True  # Mark that we are above the threshold
                        threshold_counter = 0
                        
                        absolute_laptime = self.laptime_counter
                        self.absolute_laptimes.append(absolute_laptime)

                    elif rssi_value < ADC_threshold and over_threshold:
                        over_threshold = False  # RSSI is now below the threshold
                    else:
                        threshold_counter += 1

            return self.all_rssi_values, self.absolute_laptimes
            
        except FileNotFoundError:
            print(f"File not found: {self.file_path}")
            return [], []
        except Exception as e:
            print(f"An error occurred: {e}")
            return [], []

    def display_laptimes(self):
        """Prints absolute and relative lap times."""
        print(f"Lap Timestamps: {self.absolute_laptimes}")
        print(f"Number of Datapoints: {self.laptime_counter}")

        # Calculate and print relative laptimes
        print("Relative Laptimes:")
        for i in range(1, len(self.absolute_laptimes)):
            lap_time_ticks = self.absolute_laptimes[i] - self.absolute_laptimes[i - 1]
            lap_time_minutes = lap_time_ticks // (60 * 1000)
            lap_time_seconds = (lap_time_ticks // 1000) % 60
            lap_time_milliseconds = lap_time_ticks % 1000

            # Format the lap time
            lap_time_str = f"{lap_time_minutes:02}:{lap_time_seconds:02}:{lap_time_milliseconds:03}"
            print(f"Lap {i} time: {lap_time_str}")

    def plot_rssi_values(self):
        """Plots the RSSI values against the lap time counter."""
        if len(self.all_rssi_values) == self.laptime_counter:
            plt.plot(range(1, self.laptime_counter + 1), self.all_rssi_values, label='Raw RSSI Values')
            plt.xlabel('Laptime Counter')
            plt.ylabel('RSSI Values')
            plt.title('Raw RSSI Values vs. Laptime Counter')
            plt.legend()
            plt.show()
        else:
            print("Error: The dimensions of laptime_counter and all_rssi_values do not match.")

# Usage
file_path = input("Enter the path to the CSV file: ") #rssidata.csv provided in the same test folder
lap_timer_tester = LapTimerTester(file_path)
lap_timer_tester.read_csv_file()
lap_timer_tester.display_laptimes()
lap_timer_tester.plot_rssi_values()
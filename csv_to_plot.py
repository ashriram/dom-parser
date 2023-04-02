import argparse
import json
import matplotlib.pyplot as plt

# Parse command-line arguments
parser = argparse.ArgumentParser(description='Plot Google Benchmark output JSON')
parser.add_argument('-file', type=str, help='JSON filename', required=True)
args = parser.parse_args()

# Load data from JSON
with open(args.file) as f:
    data = json.load(f)

# Create a plot
fig, ax = plt.subplots()
ax.set_title('Benchmark Results')
ax.set_xlabel('Run Name')
ax.set_ylabel('Real Time (ns)')

# Set the color of all bars to blue
color = 'b'

# Plot data for each benchmark
for item in data['benchmarks']:
    x = item['run_name']
    y = item['real_time']
    ax.bar(x, y, width=0.5, color=color)

# Remove the legend
ax.legend().remove()

# Show the plot
plt.show()
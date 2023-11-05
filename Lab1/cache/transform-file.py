# Import the csv module
import csv

# Define the input and output file names
input_file = "./input.csv"
output_file = "./output.csv"

# Open the input file in read mode
with open(input_file, "r") as input:
    # Create a csv reader object
    reader = csv.reader(input, delimiter=";")
    # Read the first row as the header
    header = next(reader)
    # Append the l1cache column name to the header
    header.append("l1cache")
    # Open the output file in write mode
    with open(output_file, "w") as output:
        # Create a csv writer object
        writer = csv.writer(output, delimiter=";")
        # Write the header to the output file
        writer.writerow(header)
        # Initialize the l1cache value
        l1cache = None
        # Loop through the remaining rows in the input file
        for row in reader:
            # Check if the row contains the l1cache value
            if row[0].startswith("l1cache-sets="):
                # Extract the l1cache value from the row
                l1cache = row[0].split("=")[1]
            else:
                # Append the l1cache value to the row
                row.append(l1cache)
                # Write the row to the output file
                writer.writerow(row)

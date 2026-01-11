- Memory segmentation:
    - Experiment with coloring memory in hexview by what instructions access them.
        - Build a graph of instructions and use force directed layout to assign colors.
            - Connect nearby instructions to help clustering.
            - Connect instructions to their return addresses.

- Statistical analysis:
    - Experiment with a live sampling profiler.
        - Use heat maps for sampled register values.
        - Instruction view (RIP only) and memory view (other registers).
        - Use mixture of guassian modeling to detect anomalous activity.
        - Bin addresses heirarchically into regions.
        - Color according to entropy weighted surprisal.

- 3D Plotting:
    - Display a grid of plots with different strides for quick visual identification.
    - Update and watch data file for live visualization.
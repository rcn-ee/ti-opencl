from __future__ import print_function
class plot():

    """Labels Each rectangle in barchart
    @rects
        ax.bar object
    @ax
        ax object
    """
    @staticmethod
    def autolabel(rects, ax):
        """ Attach a text label above each bar displaying its height """
        for rect in rects:
            height = rect.get_height()
            ax.text(rect.get_x() + rect.get_width() / 2., .10 * height,
                    '%d' % int(height),
                    ha='center', va='bottom', fontsize=4)


    """Graphs Profiling Data for Kernel using Matplotlib, pandas, and seaborn

    Args:
        kernel: kernel object
    """
    @staticmethod
    def plot(kernel, a, b, title):
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        import numpy as np
        import os
        from matplotlib import cm
        import pandas as pd

        """ Setup horizontal bar chart """
        df = pd.DataFrame(a, index=b)
        df = df.astype(float)
        ax = df.plot(kind='barh', legend=False, width=.8, figsize=(10, 20))
        plt.title(title, fontweight="bold")

        """setup legend for cores, axis, and backgound color """
        patches, labels = ax.get_legend_handles_labels()
        legend = ax.legend(patches, labels, loc='best', fancybox=True, title='Core Number',frameon=True,shadow=True)
        frame = legend.get_frame()
        frame.set_facecolor('lightgray')
        ax.set_ylabel('Profiling Events', weight='bold')
        ax.set_xlabel('Value', weight='bold')
        ax.set_facecolor("lightgray")

        """save to figureX.png, where X is the number of current plots in the folder.
        Used to avoid naming collisions. """
        plot_num = 1
        while os.path.exists('profiling/' + title + "_" + str(plot_num) + '.png'):
            plot_num += 1

        """save figure"""
        plt.savefig('profiling/' + title + "_" + str(plot_num) + '.png', bbox_inches='tight', dpi=200,facecolor='lightgray')

    @staticmethod
    def form_graph(kernel):
        import numpy as np
        """ Form Numpy Matrix of [event number][core]"""
        a = np.array(kernel.core_to_event_values)
        a = a.transpose()

        # default title
        title = kernel.name + "_Profiling"
        events = kernel.list_events_by_name

        """ if profiling all 50 events """
        if kernel.is_profiling_all_events:
            title = kernel.name + "_Memory"
            # plot first 30 memory events
            plot.plot(kernel, a[:30][:], events[:30], title)
            # plot 20 stall events
            a = a[30:][:]
            events = events[30:]
            title = kernel.name + "_Stall_"
        plot.plot(kernel, a, events, title)

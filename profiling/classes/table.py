from __future__ import print_function


class table():
    """Prints core data for event in HTML

    Args:
        kernel: kernel object
        outfile: file to write HTML table to
        event_index: index for event data in kernel.core_to_event_values
    """
    @staticmethod
    def print_core_data(kernel, outfile, event_index):
        for core in range(kernel.num_cores):
            val_with_commas = format(long(kernel.core_to_event_values[core][event_index]), ",")
            print('<td style="text-align: right;">', val_with_commas, '</td>', file=outfile)


    """Prints the percent values of stall cycle events. If not a stall cycle event,
    will print N\A

    Args:
        kernel: kernel object
        outfile: file to write HTML table to
        event_index: index for event data in kernel.core_to_event_values
    """
    @staticmethod
    def print_percent(kernel, outfile, event_name, event_index):
        if 'STALL' in event_name and kernel.total_stalls>0:
            event_stalls = 0
            for core in range(kernel.num_cores):
                event_stalls += int(kernel.core_to_event_values[core][event_index])
            percent = float(event_stalls) * 100 / kernel.total_stalls
            print('<td style="text-align: right;">', "%.2f" % percent, '%', '</td>', file=outfile)
        else:
            print('<td style="text-align: center;">', 'N\A', '</td>', file=outfile)

    """Determines the appropriate row label in order to alternate adjacent rows and to separate
    memory event from stall events in the table

    Args:
        event_name:   name of profiled event
        event_index:  event's index in kernel.list_events_by_name

    Returns:
        row_label:    html string of row with appropriate color
    """
    @staticmethod
    def get_row_label(event_name, event_index):
        row_label = '<tr style=" background-color: #2ECC71;">'
        if 'STALL' in event_name:
            row_label = '<tr style=" background-color: #CD6155;">'
            if event_index % 2 == 0:
                row_label = '<tr style=" background-color: #F5D7B1;">'
        elif event_index % 2 == 0:
            row_label = '<tr style=" background-color: #ABEBC6;">'
        return row_label

    """Prints event data for kernel in HTML

    Args:
        kernel: kernel object
        outfle: file to write HTML table to
    """
    @staticmethod
    def print_events(kernel, outfile):
        for event_index, event_name in enumerate(kernel.list_events_by_name):
            row_label = table.get_row_label(event_name, event_index)
            print(row_label, file=outfile)
            print('<td style="text-align: left;">', event_name, '</td>', file=outfile)
            table.print_core_data(kernel, outfile, event_index)
            table.print_percent(kernel, outfile, event_name, event_index)
            print('</tr>', file=outfile)


    """Prints kernel data for kernels in HTML

    Args:
        kernel: kernel object"""
    @staticmethod
    def form_table(kernel):
        with open('profiling/aetdata.html', 'a') as outfile:
            """print header"""
            print('<h2>', 'Profiling Information for ', kernel.name, 'kernel', '</h2>', file=outfile)
            print('<table style="width:100%;border=1;frame=hsides;rules=rows" border=1>', file=outfile)
            print('<tr>', file=outfile)
            print('<th>Event Name</th>', file=outfile)
            for core in range(kernel.num_cores):
                print('<th style="text-align: Center;">Core ', str(core), '</th>', file=outfile)
            print('<th>Percent</th>', file=outfile)
            print('</tr>', file=outfile)

            """ print events"""
            table.print_events(kernel, outfile)
            print('</table>', file=outfile)

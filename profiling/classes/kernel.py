"""Class Used to Contain Profile Data for each Kernel"""
class kernel_data(object):
    """Class Used to Contain Profile Data for each Kernel

    Attributes:
        name: name of kernel
        event_list: list of event objects profiled during kernel execution
        list_events_by_name: list of event names profiled during kernel execution
        core_to_event_values: Matrix where each index is defined as [core][event_index]
                                and where values = event value. Event index is the index
                                of the event in list_events_by_name.
        num_cores: number of cores in kernel
        total_stalls: Total number of stalls during kernel execution. Used to calculate
                        percent stalls for each event.
    """
    def __init__(self):
        self.name = ""
        self.list_events_by_name = []
        self.core_to_event_values = []
        self.num_cores = 0
        self.event_list = []
        self.total_stalls = 0

    """Forms a JSON of all kernel data for aggregated_kernel

    Args:
        kernel: kernel object"""
    def make_kernel_json(self):
        kernel_info = {}
        kernel_info["Kernel Data"] = []
        kernel_info["Kernel Name"] = self.name

        """ form JSON of event data and append to event_label"""
        for event in self.event_list:
            event_data = event.make_event_info_json()
            event_label = {}
            event_label['Event Data'] = event_data
            kernel_info["Kernel Data"].append(event_label)
        return kernel_info

    """ Removes Duplicate Events from self.event_list """
    def remove_duplicate_events(self):
        new_events_list = []

        events = set()
        for event in self.event_list:
            if event.event_1_name not in events:
                events.add(event.event_1_name)
                new_events_list.append(event)

        self.event_list = new_events_list

    """ Determines if kernel has profile information for all 50 events """
    def det_if_profiling_all_events(self):
        self.is_profiling_all_events = len(self.list_events_by_name) == 50

    """ initializes core_to_event matrix """
    def initialize_core_to_event(self):
        self.num_cores = len(self.event_list[0].core_data_list)
        for i in range(self.num_cores):
            self.core_to_event_values.append([])



    """ Updates event_core_to_event matrix according to profile data.

    Args:
        event: event object
    """
    def update_event_core_data(self, event):
        event_type = int(event.event_type)
        for core in event.core_data_list:
            """ Record appropriate counter value for event """
            if event_type == 1:
                self.core_to_event_values[core.core_number].append(core.counter1_diff)
            if event_type == 2:
                self.core_to_event_values[core.core_number].append(core.counter0_diff)
                event_number2 = int(event.event_number2)
                if event_number2 >= 0:
                    self.core_to_event_values[core.core_number].append(core.counter1_diff)

    """ Aggregates all profile information into a matrix, removing unused counter values."""
    def form_profiling_matrix(self):
        self.initialize_core_to_event()
        self.remove_duplicate_events()

        """ list_event_by_name is a list of event names
            core_to_event_values is a matrix of [core number][event index]"""
        for event in self.event_list:
            """ update self.list_events_by_name with profiled events"""
            self.list_events_by_name.append(event.event_1_name)
            event_type = int(event.event_type)

            # if event is profiling 2 mem events, append data for second event
            if event_type == 2:
                event_number2 = int(event.event_number2)
                if event_number2 >= 0:
                    self.list_events_by_name.append(event.event_2_name)
            self.update_event_core_data(event)


    """ counts total number of stall cycles """
    def count_total_stall_cycles(self):
        for event in self.event_list:
            # event is stall cycles in cpu pipeline, record stalls
            if (int(event.event_number1)==0) and (int(event.event_type)==1):
                self.total_stalls = event.sum_event_stalls()











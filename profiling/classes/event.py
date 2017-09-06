from event_list import *
from core import core_data

class event_data(object):
    """Class used to contain all event data for profile

    Attributes:
        core_data_list: list of core data for event
        start_index:    starting index in string where event data starts
        last_index:     ending index in string where event data ends

        event_type:     type of event. 0 is stall cycle event, 1 is 1 memory event,
                            2 is 2 memory events
        event_number1:  event number according to AET library for 1st event
        event_number2:  event number according to AET library for 2nd event
        event_1_name:   name of event according to AET library for 1st event
        event_2_name:   name of event according to AET library for 2nd event

        counter0_label: Label for counter0 values (based off counter0 hardware counter)
        counter1_label: Label for counter1 values

        stall_cycle_threshold:  Stall Cycle Threshold to count stall cycles
    """

    def __init__(self):
        self.core_data_list = []
        self.start_index = -1
        self.last_index = -1

        self.event_type = -1
        self.event_number1 = -1
        self.event_number2 = -1
        self.event_1_name = ""
        self.event_2_name = ""

        self.counter0_label = ""
        self.counter1_label = ""

        self.stall_cycle_threshold = -1

    """Updates event names that were profiled in {@code event1}
    @param event1
        event object"""
    def update_event_names(self):
        """Update Labels for the two counters depending on the event data"""
        # if memory events
        if int(self.event_type) == 2:
            event_number1 = int(self.event_number1)
            """access event name by number from event_list dictionary"""
            self.event_1_name = event_list.mem_events[event_number1]
            self.counter0_label = self.event_1_name
            event_number2 = int(self.event_number2)
            if event_number2 >= 0:
                self.event_2_name = event_list.mem_events[event_number2]
                self.counter1_label = self.event_2_name
            else:
                self.counter1_label = "Unused Counter"
        # if stall cycle event
        elif int(self.event_type) == 1:
            event_number1 = int(self.event_number1)
            self.event_1_name = event_list.stall_events[event_number1]
            self.counter0_label = "Threshold Counter"
            self.counter1_label = self.event_1_name

    """Finds all core data for kernel and updates kernel.core_data_list

    Args:
        kernel: kernel object that will contain all core data during kernel execution
        lines: raw profile dump. This stream has a seperator ~~~~End Core which divides core data
    """
    def process_core_data(self, lines):
        """Step through the raw data of lines in steps of 1 until we find the
        "~~~~End Core" separator. Then, grab core_data by accessing lines preceding
        the separator"""
        core_index = self.start_index
        while core_index <= self.last_index:
            if "~~~~End Core" in lines[core_index]:
                core_data1 = core_data()
                core_data1.core_name = "Core " + lines[core_index - 3]
                core_data1.core_number = int(lines[core_index - 3])
                core_data1.counter0_diff = lines[core_index - 2]
                core_data1.counter1_diff = lines[core_index - 1]
                self.core_data_list.append(core_data1)  # update list with new core_data
            core_index += 1

    """Forms a JSON of Core information

    Args:
        kernel: kernel object
    Returns:
        cores:  JSON object containing core profiling information"""
    def make_core_info_json(self):
        cores = []
        """form list of core data for the current kernel"""
        for core in self.core_data_list:
            core_info = {}
            core_info['core_number'] = core.core_name
            core_info['counter0_change'] = core.counter0_diff
            core_info['counter1_change'] = core.counter1_diff
            cores.append(core_info)
        return cores

    """Forms a JSON of event_data

    Returns:
        event_data: JSON event data"""
    def make_event_info_json(self):
        event_data = {}
        event_data['counter 0 label'] = self.counter0_label
        event_data['counter 1 label'] = self.counter1_label
        event_data['stall cycle_threshold'] = self.stall_cycle_threshold
        event_data['event type'] = self.event_type

        cores = self.make_core_info_json()
        event_data['Core Information'] = cores
        return event_data

    def sum_event_stalls(self):
        sum_event_stalls = 0
        for core in self.core_data_list:
            sum_event_stalls += int(core.counter1_diff)
        return sum_event_stalls

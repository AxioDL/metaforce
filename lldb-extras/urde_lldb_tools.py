import lldb

class CMatrix3f_Provider:

    def __init__(self, valobj, dict):
        self.valobj = valobj

    def num_children(self):
        return 3

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        try:
            m = self.valobj.GetChildMemberWithName('m')
            return m.GetChildAtIndex(index)
        except:
            return None

    def has_children(self):
        return True

class CMatrix4f_Provider(CMatrix3f_Provider):

    def num_children(self):
        return 4

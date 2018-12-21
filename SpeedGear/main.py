import sys
from PyQt5.QtWidgets import *
import psutil
from ctypes import *
dll = windll.LoadLibrary("HookDll.dll")
StartHook = getattr(dll, "StartHook")
setSpeed = getattr(dll, "setSpeed")
StopHook = getattr(dll, "StopHook")

class SigSlot(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self)
        self.setWindowTitle('SpeedGear')

        list_all_proc = QListWidget(self)
        list_gear_proc = QListWidget(self)
        txt_pid = QTextEdit(self)
        txt_speed = QTextEdit(self)
        lbl_pid = QLabel(self)
        lbl_pid.setText('进程 ID')
        lbl_speed = QLabel(self)
        lbl_speed.setText('变速倍率')

        list_all_proc.currentRowChanged.connect(self.itemChanged)

        grid = QGridLayout()
        btn_start_gear = QPushButton(self)
        btn_start_gear.setText('将进程加入变速')
        btn_stop_gear = QPushButton(self)
        btn_stop_gear.setText('停止进程的变速')
        btn_set_gear = QPushButton(self)
        btn_set_gear.setText('设置变速')
        btn_refreash = QPushButton(self)
        btn_refreash.setText('刷新左面列表')

        btn_start_gear.clicked.connect(self.startGear)
        btn_stop_gear.clicked.connect(self.stopGear)
        btn_set_gear.clicked.connect(self.setGear)
        btn_refreash.clicked.connect(self.refreash)

        grid.addWidget(list_all_proc, 0, 0, 8, 1)
        grid.addWidget(lbl_pid, 0, 1, 1, 1)
        grid.addWidget(txt_pid, 1, 1, 1, 1)
        grid.addWidget(btn_start_gear, 2, 1, 1, 1)
        grid.addWidget(btn_stop_gear, 3, 1, 1, 1)
        grid.addWidget(btn_refreash, 4, 1, 1, 1)
        grid.addWidget(lbl_speed, 5, 1, 1, 1)
        grid.addWidget(txt_speed, 6, 1, 1, 1)
        grid.addWidget(btn_set_gear, 7, 1, 1, 1)
        grid.addWidget(list_gear_proc, 0, 2, 8, 1)

        self.allpids = psutil.pids()
        self.pids = list()
        self.pid_map = dict()
        self.pid_in_gear = list()
        
        for pid in self.allpids:
            p = psutil.Process(pid)
            list_all_proc.addItem('%s (%s)' % (p.name(), pid))
            self.pids.append(pid)
            self.pid_map[pid] = p.name()

        self.setLayout(grid)
        self.resize(700, 500)
        self.list_all_proc = list_all_proc
        self.list_gear_proc = list_gear_proc
        self.txt_pid = txt_pid
        self.txt_speed = txt_speed

    def itemChanged(self, row):
        self.txt_pid.setText(str(self.pids[row]))

    def startGear(self):
        pid = self.txt_pid.toPlainText()
        print('Start on %s' % pid)
        self.pid_in_gear.append(int(pid))
        StartHook(c_ulong(int(pid)))
        self.refreshGearList()

    def stopGear(self):
        pid = self.txt_pid.toPlainText()
        print('Stop on %s' % pid)
        StopHook(c_ulong(int(pid)))
        try:
            self.pid_in_gear.remove(int(pid))
        except:
            pass
        self.refreshGearList()

    def setGear(self):
        speed = self.txt_speed.toPlainText()
        print('Change speed to %s' % speed)
        setSpeed(c_double(float(speed)))

    def refreshGearList(self):
        self.list_gear_proc.clear()
        for item in self.pid_in_gear:
            self.list_gear_proc.addItem("%s (%s)" % (self.pid_map[item], item))

    def refreash(self):
        self.list_all_proc.clear()
        self.allpids = psutil.pids()
        for pid in self.allpids:
            p = psutil.Process(pid)
            self.list_all_proc.addItem('%s (%s)' % (p.name(), pid))
            self.pids.append(pid)
            self.pid_map[pid] = p.name()
        
app = QApplication(sys.argv)
qb = SigSlot()
qb.show()
sys.exit(app.exec_())

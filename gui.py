#!/bin/python
import numpy as np
from SimulationInfo import SimulationInfo
import sys
import threading as th
import time

Vector = np.array

from PySide6.QtCore import Qt, Slot, QEvent, QRect, QPoint, QTimer, QObject
from PySide6.QtWidgets import (QApplication, QLabel, QPushButton,
							   QVBoxLayout, QWidget)
from PySide6.QtGui import (QGuiApplication, QPainter, QBackingStore, QWindow, QGradient, QColor)
from __feature__ import snake_case, true_property


class RasterWindow(QWindow):
	def __init__(self, parent, simulation):
		super().__init__(parent)
		self.m_backingStore = QBackingStore(self)

		self.sim = simulation
		self.new_values = True
		self.rendering = False

		self.set_geometry(200, 200, 600, 600)


	def render(self, painter):
		(position, velocity, force, mass) = self.sim.buffer()
		painter.draw_text(QRect(0, 0, self.width, self.height), Qt.AlignLeft | Qt.AlignTop, "0")
		for i in range(position.shape[0]):
			x = (position[i][0] + self.sim.area) * self.width // (2*self.sim.area)
			y = (position[i][1] + self.sim.area) * self.height // (2*self.sim.area)
			painter.draw_ellipse(QPoint(x, y), 3, 3)
		print('rendered positions')
	
	def event(self, event):
		if event.type() == QEvent.Type.UpdateRequest or event.type() == QEvent.Type.Show:
			self.render_now()
			return True
		return super().event(event)

	def render_later(self):
		if not self.rendering:
			self.event(QEvent(QEvent.UpdateRequest))
	
	def timer_event(self):
		print('timer')
		if self.new_values:
			self.new_values = False
			self.render_later()

	def render_now(self):
		self.rendering = True
		if not self.is_exposed():
			return

		rect = QRect(0, 0, self.width, self.height)
		self.m_backingStore.begin_paint(rect)
		device = self.m_backingStore.paint_device()
		painter = QPainter(device)
		painter.set_render_hint(QPainter.RenderHint.Antialiasing)

		painter.fill_rect(0, 0, self.width, self.height, QGradient.NightFade)
		self.render(painter)

		painter.end()
		self.m_backingStore.end_paint()
		self.m_backingStore.flush(rect)

		self.rendering = False

	def resize_event(self, resizeEvent):
		self.m_backingStore.resize(resizeEvent.size())
		self.render_now()

	def expose_event(self,event):
		if self.is_exposed():
			self.render_now()


if __name__ == "__main__":
	app = QGuiApplication(sys.argv)

	simulation = SimulationInfo()

	window = RasterWindow(None, simulation)
	window.show()
	window.render_later()

	timer = QTimer()
	timer.timeout.connect(window.timer_event)
	timer.start(1000 * 1/simulation.fps)

	def task():
		time.sleep(3.00)
		while True:
			t1 = time.time()

			print('updating')
			simulation.update()
			window.new_values = True

			t2 = time.time()
			sl = (1/simulation.fps) - (t2 - t1)

			if sl > 0:
				time.sleep(sl)
			else:
				print('######## - could not hold frame rate - #######')


	thread = th.Thread(target=task)

	thread.start()

	sys.exit(app.exec())
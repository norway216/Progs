#include "CineBuffer.h"

CineBuffer::CineBuffer(QObject *parent) : QObject(parent) {}
void CineBuffer::push(const QImage &image) { if (m_frames.size() >= m_capacity) m_frames.removeFirst(); m_frames.push_back(image); m_currentIndex = m_frames.size() - 1; emit changed(); emit currentIndexChanged(); }
QImage CineBuffer::frameAt(int index) const { if (index < 0 || index >= m_frames.size()) return {}; return m_frames[index]; }
int CineBuffer::size() const { return m_frames.size(); }
int CineBuffer::currentIndex() const { return m_currentIndex; }
void CineBuffer::clear() { m_frames.clear(); m_currentIndex = 0; emit changed(); emit currentIndexChanged(); }
void CineBuffer::setCurrentIndex(int index) { if (m_frames.isEmpty()) index = 0; else index = qBound(0, index, m_frames.size() - 1); if (m_currentIndex == index) return; m_currentIndex = index; emit currentIndexChanged(); }

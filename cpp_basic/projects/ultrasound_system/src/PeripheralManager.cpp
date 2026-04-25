#include "PeripheralManager.h"

PeripheralManager::PeripheralManager(QObject *parent) : QObject(parent) {}
bool PeripheralManager::probeConnected() const { return m_probeConnected; }
QString PeripheralManager::probeName() const { return m_probeName; }
void PeripheralManager::toggleProbe() { m_probeConnected = !m_probeConnected; m_probeName = m_probeConnected ? "C5-2 Convex" : "None"; emit probeChanged(); emit logMessage(m_probeConnected ? "PeripheralManager: probe inserted, loading probe preset" : "PeripheralManager: probe removed, releasing imaging resources"); if (m_probeConnected) emit probeInserted(); else emit probeRemoved(); }
void PeripheralManager::printImage() { emit logMessage("PrinterManager::printImage(): simulated print job queued"); }
void PeripheralManager::saveToUsb() { emit logMessage("DiskManager::saveToUSB(): simulated FAT32 export success"); }

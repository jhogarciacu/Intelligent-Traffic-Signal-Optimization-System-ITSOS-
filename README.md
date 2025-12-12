# Intelligent-Traffic-Signal-Optimization-System-ITSOS-
AI-powered traffic light optimization system using YOLO vehicle detection, AWS InfluxDB dashboards, and ESP32-based IoT control for real-time intersection management.

### 🚦 1. Project Overview
Urban intersections often suffer from inefficient traffic flow due to fixed-timing signal systems. These systems do not account for real-time congestion, causing unnecessary waiting times, increased emissions, and traffic buildup.

This project introduces an AI-powered traffic optimization system that dynamically adjusts traffic light timing based on live vehicle detection. Using a YOLO-based model running on an edge gateway, the system analyzes traffic conditions and prioritizes lanes with higher congestion. Data is monitored through an AWS-hosted dashboard, and traffic signals are controlled via ESP32-C6 IoT devices.

This repository is structured and written to demonstrate professional engineering practices, making it suitable for academic presentation, portfolio showcasing, and industry-level evaluation.

### 2. Key Features
-  Real-time AI vehicle detection using YOLO.
-  Adaptive traffic light timing based on congestion analysis.
-  Edge computation ensuring minimal latency.
-  MQTT-based communication between gateway and ESP32-C6 controllers.
-  Time-series data storage in AWS InfluxDB.
-  Real-time dashboard visualization of vehicle counts, congestion, and system decisions.
-  Low-cost and scalable IoT deployment for smart-city applications.

### 3.System Architecture
Core components:
-  Edge Gateway → Runs detection model, decides priority, sends metrics & commands.
-  ESP32-C6 Devices → Control traffic light timing using commands received via MQTT.
-  AWS Server (InfluxDB) → Stores traffic metrics and powers the monitoring dashboard.
-  MQTT Broker → Communication backbone between gateway and controllers.

Data Flow Summary:
-  Gateway captures an image frame.
-  YOLO model detects vehicles and their counts.
-  Decision engine assigns traffic priority.
-  Gateway sends metrics → InfluxDB.
-  Gateway sends control messages → ESP32-C6
-  ESP32-C6 adjusts green/red light timing.
-  Dashboard displays real-time and historical data.

### Visual Output of the System
<img width="1280" height="719" alt="image" src="https://github.com/user-attachments/assets/bd019599-192a-43ac-a780-872982dafab0" />

<img width="1600" height="903" alt="image" src="https://github.com/user-attachments/assets/6a472892-4f60-4eab-baf9-bd70f636b93e" />





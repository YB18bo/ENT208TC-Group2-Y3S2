# Watt's Up (DormGuard) — User Roles

Three roles use the Watt's Up (DormGuard) system. Every feature belongs to one or more of these roles.

## Role overview

| Role | Who they are | Typical number per group |
|---|---|---|
| **Dorm Resident Student** | An undergraduate student living in a 4-6 person shared dorm. | 4–6 per room |
| **Building Manager** | Staff responsible for the daily safety and operation of a specific dormitory building. | 1–2 per building |
| **Property Manager** | Campus administration overseeing sustainability and utility costs across multiple buildings. | 1–3 per campus |

---

## Dorm Resident Student

**Goal:** Save energy and reduce electricity bills effortlessly without changing daily habits.

**Can do:**
- View their own room's energy-saving status and daily saved energy (kWh).
- View the "Last 3 Auto-Saving Actions" log for their specific room.
- Toggle "Smart Power Zones" settings on/off (e.g., Desk Zone, Bedside Zone).
- Receive real-time push notifications when power is automatically cut off.
- Manually override the auto-shutoff to restore power.

**Cannot do:**
- View the real-time power consumption or occupancy status of any other dorm room.
- Access the Building Manager's floor plan dashboard.
- Change the system-wide threshold for auto-shutoff (e.g., changing the 15-minute timer or 300 lux threshold).

---

## Building Manager

**Goal:** Ensure building safety and immediately intervene in abnormal or high-risk electricity usage.

**Can do:**
- Access the Admin Dashboard with a color-coded floor plan view (green, yellow, red).
- View real-time power curves and last occupancy timestamps for any room in their assigned building.
- Receive real-time "High Risk" alerts (e.g., standby >5W, night consumption >200W).
- Log intervention actions in the "Action Log" tab (e.g., "Inspected room - removed heater").
- View the daily "Top 5 High-Consumption Rooms" leaderboard for their building.

**Cannot do:**
- View or change a student's personal app settings (e.g., Smart Power Zones).
- Remotely cut off or restore power to a specific device (they can only monitor and physically inspect).
- Access high-level property management analytics across different buildings.

---

## Property Manager

**Goal:** Assess overall campus performance, identify waste hotspots, and motivate students through data.

**Can do:**
- Access the "Energy Insights" module on the Property Management Dashboard.
- Drill down analytics from Building → Floor → Room with hourly energy heatmaps.
- Automatically receive and view daily summary reports (target achievement, best rooms, personalized recommendations).
- Export PDF reports with charts and actionable recommendations.
- Manage the "Public Display Mode" content (weekly leaderboards and eco tips).

**Cannot do:**
- Receive granular, real-time alerts for individual room anomalies (their focus is on daily/weekly trends, not minute-by-minute firefighting).
- Log physical intervention actions for specific rooms (this is the Building Manager's job).

---

## Rules for the AI agent

Apply these to every feature you build:

1. **Strict Data Isolation for Students.** `user.role === 'student'` must ONLY have access to data where `room_id` matches their own. Never expose another room's data to a student.
2. **Distinct Dashboards.** The system has three distinct frontend interfaces: Mobile App (Students), Admin Dashboard (Building Managers), and Analytics Dashboard (Property Managers). Do not mix UI components between them.
3. **Action Logs are permanent.** When a Building Manager records an intervention in the Action Log, it is append-only. Do not provide edit or delete APIs for these logs.
4. **Read-only Monitoring for Managers.** Managers can see power curves and alerts, but do not generate UI buttons for managers to remotely control student hardware relays. The hardware acts autonomously.
5. **When in doubt about permissions, ask before generating.** Do not guess.

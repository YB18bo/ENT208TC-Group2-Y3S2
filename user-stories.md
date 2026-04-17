# Watt's Up (DormGuard) — User Stories

---

## Story 1 — Dorm Resident Student (Auto Power-Off)

**As a dorm resident student**, I want the system to automatically cut power to non-essential devices when the room is unoccupied, so that I can save energy without changing my daily habits.

**✅ Acceptance Criteria:**

- [ ] When both the infrared (PIR) sensor and ambient light sensor confirm the room has been unoccupied for ≥15 minutes and ambient light is ≥300 lux, the system must automatically disable designated outlets (e.g., desk lamp, phone charger) within 30 seconds.
- [ ] Immediately after power cutoff, a push notification must appear in the student’s mobile app within 2 seconds: “Power off: Desk lamp & USB outlet (saved 0.18 kWh)”.
- [ ] If the student re-enters the room within 5 minutes of cutoff, the system should automatically restore power to previously active devices (with an option for manual override).
- [ ] In a 3-day pilot: ≥80% of students report noticing the auto-shutoff feature and agree with the statement: “It helped me save energy without requiring extra effort.”

**📱 UI Description (for designers):**

* **Home screen** → [Energy-Saving Status] card (e.g., “✅ Idle mode active — Saved 0.2 kWh today”)
* **Tap interaction** → View “Last 3 Auto-Saving Actions” (device name, time, energy saved)
* **Settings page** → Toggle “Smart Power Zones” on/off (e.g., Desk Zone, Bedside Zone, AC-dedicated outlet)

---

## Story 2 — Dormitory Building Manager (High-Risk Alert)

**As a dormitory building manager**, I want the system to provide real-time alerts for abnormal electricity usage patterns, so I can proactively intervene before issues escalate—reducing both energy waste and safety hazards.

**✅ Acceptance Criteria:**

- [ ] The system must identify and flag either of the following as “High Risk” within 1 minute:
  - A room consuming >5 W in standby mode while unoccupied (i.e., >1.5× the floor average).
  - Continuous high-power consumption (>200 W) between midnight and 6:00 AM.
- [ ] Each alert must include: room number, current power draw, duration, and inferred device type (e.g., “Likely electric heater”).
- [ ] The manager dashboard must generate a daily “Top 5 High-Consumption Rooms” leaderboard, ranked by energy intensity.
- [ ] In a 1-week pilot: ≥90% of “High Risk” alerts are verified by staff as true positives (not false alarms).

**📱 UI Description (for designers):**

* **Admin Dashboard** → Floor plan view (color-coded by consumption level: green ≤ average, yellow 1–1.5×, red >1.5×)
* **Click any room** → Pop-up detail panel showing: real-time power curve, last occupancy timestamp, and alert history
* **“Action Log” tab** → Allows recording interventions (e.g., “Room 302 inspected — removed unauthorized heating pad”)

---

## Story 3 — Property Manager (Daily Reporting)

**As a property manager**, I want to receive actionable daily energy-saving reports for each dorm unit so I can assess performance, identify improvement opportunities, and use data to motivate all residents to participate in energy conservation.

**✅ Acceptance Criteria:**

- [ ] A summary report must be automatically generated and delivered via email/SMS every day at 8:00 AM, including:
  - Total building-wide energy saved that day vs. target achievement rate.
  - Top 3 best-performing dorm rooms in energy saving.
  - One personalized recommendation (e.g., “B4 Building reduced standby power by 40%—consider promoting their ‘power-off-when-leaving’ habit!”).
- [ ] The report must clearly flag “Waste Hotspots”: rooms where electricity consumption during unoccupied periods exceeds 30% of their total usage.
- [ ] A weekly “Energy-Saving Leaderboard” must be displayed in public areas (e.g., bulletin boards or lobby digital screens), ranked by percentage reduction in standby power consumption (not total energy use) to ensure fair and comparable evaluation.
- [ ] In the first month of pilot deployment: ≥70% of surveyed residents report having seen the leaderboard and state they adjusted their behavior as a result (e.g., “I now unplug chargers more proactively”).

**📱 UI Description (for designers):**

* **Property Management Dashboard** → “Energy Insights” Module:
  * Daily Summary Card (date, total building energy saved, “Room of the Week”)
  * Drill-down analytics: Building → Floor → Room → Hourly energy heatmap (overlaid with occupancy status)
  * One-click export button to generate a PDF report with charts and actionable recommendations
* **Public Display Mode** → Rotating showcase of “This Week’s Energy Saver” and “Eco Tips” (e.g., “Unplug idle chargers—save 0.05 kWh per day!”)

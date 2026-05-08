# Energy Monitoring System — User Stories

---

## Role 1: Dorm Resident Student

**User Story:**
As a dorm resident student, I want the system to automatically cut power to non-essential devices when the room is unoccupied, so that I can save energy without changing my daily habits.

**✅ Acceptance Criteria:**

- [ ] When both the infrared (PIR) sensor and ambient light sensor confirm the room has been unoccupied for ≥15 minutes and ambient light is ≥300 lux, the system must automatically disable designated outlets (e.g., desk lamp, phone charger) within 30 seconds.
- [ ] Immediately after power cutoff, a push notification must appear in the student’s mobile app within 2 seconds: “Power off: Desk lamp & USB outlet (saved 0.18 kWh)”.
- [ ] If the student re-enters the room within 5 minutes of cutoff, the system should automatically restore power to previously active devices (with an option for manual override).
- [ ] In a 3-day pilot: ≥80% of students report noticing the auto-shutoff feature and agree with the statement: “It helped me save energy without requiring extra effort.”

**📱 UI Description (for designers):**

* **Home screen** → [Energy-Saving Status] card (e.g., “✅ Idle mode active — Saved 0.2 kWh today”)
* **Tap interaction** → Tap to view “Last 3 Auto-Saving Actions” (device name, time, energy saved)
* **Settings page** → Toggle “Smart Power Zones” on/off (e.g., Desk Zone, Bedside Zone, AC-dedicated outlet)

---

## Role 2: Company Finance Officer

**User Story:**
As a company finance officer, I want the system to provide daily cost-impact reports on energy waste across all factory units, so I can track operational efficiency, justify sustainability investments, and align energy usage with budget forecasts.

**✅ Acceptance Criteria:**

- [ ] A daily financial impact report must be automatically delivered via email at 8:00 AM, including:
  - Total electricity cost saved that day vs. baseline (in local currency);
  - Estimated monthly savings if current behavior continues;
  - List of top 3 “Cost-Leak” machines or zones (e.g., “CNC Line 2 standby cost: ¥42/day”).
- [ ] Any machine consuming >5 W in standby while idle for ≥30 minutes must be flagged as a “Cost Waste Alert” with potential annualized loss (e.g., “¥1,530/year if unchanged”).
- [ ] The system must calculate ROI for recommended retrofits (e.g., “Smart outlet upgrade for Welding Station B: Payback in 6 months”).
- [ ] In a 2-week pilot: ≥85% of reported “Cost Waste Alerts” are confirmed by operations staff as valid opportunities for savings.

**📱 UI Description (for designers):**

* **Finance Dashboard** → “Energy Cost Insights” Module:
  * Daily Summary Card: “Saved ¥287 today — on track for ¥8,600/month”
  * “Top Cost Leaks” table: Machine ID, Standby Power, Daily Cost, Annual Projection
  * “Investment Recommendations” section: Retrofit options with payback period & NPV
  * One-click export to Excel/PDF for inclusion in monthly financial reviews

---

## Role 3: Factory Workshop Supervisor

**User Story:**
As a factory workshop supervisor, I want real-time alerts and visual dashboards for abnormal or wasteful energy use on the production floor, so I can quickly correct inefficiencies, prevent equipment damage, and ensure shift teams follow energy discipline.

**✅ Acceptance Criteria:**

- [ ] The system must detect and trigger a “High-Risk Energy Event” within 1 minute if either condition occurs:
  - A machine draws >200 W during non-production hours (e.g., nights/weekends) without scheduled maintenance.
  - Standby power of any production line exceeds 10% of its average active power while idle.
- [ ] Each alert must include: machine ID, current power draw, expected idle threshold, duration, and likely cause (e.g., “Coolant pump left running”).
- [ ] The supervisor’s dashboard must show a live floor map with color-coded machine status:
  - Green: Normal idle (<5 W or within spec)
  - Yellow: Elevated standby (5–50 W)
  - Red: High-risk (>50 W or >200 W off-hours)
- [ ] In a 1-week pilot: ≥90% of red alerts lead to verified corrective actions (e.g., turning off forgotten equipment).

**📱 UI Description (for designers):**

* **Workshop Supervisor Dashboard** → Real-Time Floor View:
  * Interactive plant layout with machine icons colored by energy status
  * Click any machine → Pop-up showing: real-time power curve, last production timestamp, idle duration, and alert history
  * “Shift Energy Score” card: Compares current shift’s standby waste vs. weekly average
  * “Quick Action” button: Log intervention (“Turned off hydraulic press – saved ~1.2 kWh”)
  * Optional SMS/pager alert toggle for critical red events

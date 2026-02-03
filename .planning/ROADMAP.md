# 项目路线图

## Current Milestone: v0.1 Initial

### Phase 1: Basic Setup and Connectivity

**Goal:** Establish basic ESP32 setup, WiFi connectivity, and WebSocket communication with the Mac server.
**Depends on:** None
**Plans:** 0 plans

Plans:
- [ ] TBD (run /gsd:plan-phase 1 to break down)

**Details:**
[To be added during planning]

### Phase 2: Refactor and Advanced Connectivity

**Goal:** Refactor monolithic code, implement multi-WiFi support, and add mDNS service discovery.
**Depends on:** Phase 1
**Status:** Completed
**Plans:** 1 plans

Plans:
- [x] Refactor and Connectivity Upgrade (run /gsd:plan-phase 2 to break down)

**Details:**
- Split logic from single file into modules.
- Support list of WiFi credentials for sequential connection attempts.
- Implement mDNS to automatically discover server (e.g., hostname.local) instead of hardcoded IP.



---

## Future Milestones

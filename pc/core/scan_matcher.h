/**
 * pc/core/scan_matcher.h
 * 
 * Matches TriggerEvent stream with LaserProfile stream using FIFO logic.
 * Produces MatchedScanFrame records with detailed status tracking.
 */

#pragma once

#include <cstdint>
#include <queue>
#include <mutex>
#include <memory>
#include <vector>

// Forward declarations
struct TriggerEvent;
struct LaserProfile;

/**
 * MatchedScanFrame: Result of matching a trigger with a profile.
 */
struct MatchedScanFrame {
    enum Status {
        MATCHED,              // Trigger + profile paired successfully
        DROPPED_TRIGGER,      // Trigger was old (profile arrived before it)
        DROPPED_PROFILE,      // Profile was old (trigger arrived before it)
        ORPHAN_TRIGGER,       // Trigger timed out waiting for profile
        TIMEOUT_PROFILE       // Profile timed out waiting for trigger
    };
    
    // Matched data (valid only if status == MATCHED)
    TriggerEvent trigger;
    LaserProfile profile;
    
    // Status
    Status status;
    
    // Latency (profile.timestamp_us - trigger.tick_us)
    // Positive = profile arrived after trigger (normal)
    // Negative = profile arrived before trigger (reorder or dropped)
    int64_t latency_us = 0;
};

/**
 * ScanMatcher: FIFO-based dual-stream matcher.
 * 
 * Architecture:
 * - Trigger queue (FIFO)
 * - Profile queue (FIFO)
 * - Matcher thread continuously processes:
 *   1. Try to match front of trigger queue with front of profile queue
 *   2. If latency < 0: profile is old, drop profile
 *   3. If latency > timeout: trigger is old, drop trigger
 *   4. Otherwise: match and emit MatchedScanFrame
 * 
 * Usage:
 *   ScanMatcher matcher(100000);  // 100ms timeout
 *   matcher.addTrigger(evt);
 *   matcher.addProfile(prof);
 *   
 *   MatchedScanFrame frame;
 *   while (matcher.tryGetMatched(frame, 100)) {
 *       log(frame);
 *   }
 */
class ScanMatcher {
public:
    /**
     * Constructor.
     * @param timeout_us Maximum time to wait for profile after trigger (microseconds)
     */
    explicit ScanMatcher(uint32_t timeout_us = 100000);
    
    /**
     * Destructor.
     */
    ~ScanMatcher();
    
    /**
     * Add a trigger event to the matcher.
     */
    void addTrigger(const TriggerEvent& evt);
    
    /**
     * Add a profile to the matcher.
     */
    void addProfile(const LaserProfile& prof);
    
    /**
     * Try to get next matched frame (non-blocking or with timeout).
     * @param frame [out] Populated with MatchedScanFrame if available
     * @param timeout_ms Timeout (0 = non-blocking, -1 = infinite)
     * @return true if frame was available, false if queue empty or timeout
     */
    bool tryGetMatched(MatchedScanFrame& frame, int timeout_ms = 100);
    
    /**
     * Get current trigger queue size.
     */
    size_t triggerQueueSize() const;
    
    /**
     * Get current profile queue size.
     */
    size_t profileQueueSize() const;
    
    /**
     * Statistics for monitoring matching quality.
     */
    struct Stats {
        uint32_t matched = 0;           // Successfully matched frames
        uint32_t dropped_triggers = 0;  // Triggers rejected (too old)
        uint32_t dropped_profiles = 0;  // Profiles rejected (too old)
        uint32_t orphan_triggers = 0;   // Triggers timed out waiting for profile
        uint32_t timeout_profiles = 0;  // Profiles timed out waiting for trigger
        
        // Latency stats
        int64_t min_latency_us = INT64_MAX;
        int64_t max_latency_us = INT64_MIN;
        int64_t sum_latency_us = 0;
        uint32_t latency_samples = 0;
        
        // Helper to compute average latency
        double avgLatencyUs() const {
            return latency_samples > 0 ? (double)sum_latency_us / latency_samples : 0.0;
        }
    };
    
    /**
     * Get copy of current statistics.
     */
    Stats getStats() const;
    
    /**
     * Clear statistics counters.
     */
    void clearStats();
    
    /**
     * Get/set timeout (in microseconds).
     */
    uint32_t getTimeoutUs() const { return m_timeout_us; }
    void setTimeoutUs(uint32_t us) { m_timeout_us = us; }

private:
    // Configuration
    uint32_t m_timeout_us;
    
    // Queues
    mutable std::mutex m_queue_mutex;
    std::queue<TriggerEvent> m_trigger_queue;
    std::queue<LaserProfile> m_profile_queue;
    
    // Output queue
    mutable std::mutex m_output_mutex;
    std::queue<MatchedScanFrame> m_output_queue;
    static constexpr size_t MAX_OUTPUT_QUEUE = 1000;
    
    // Statistics
    mutable std::mutex m_stats_mutex;
    Stats m_stats;
    
    // Matcher thread
    std::thread m_matcher_thread;
    std::atomic<bool> m_running{false};
    
    // Matcher thread main loop
    void matcherThreadMain();
    
    // Helper: try to match one pair
    bool tryMatchOne(MatchedScanFrame& frame);
};

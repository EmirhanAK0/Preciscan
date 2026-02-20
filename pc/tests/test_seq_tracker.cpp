#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "seq_metrics.h"

static void feed(SeqTracker& t, std::initializer_list<uint32_t> seqs) {
    for (uint32_t s : seqs) t.update(s);
}

TEST_CASE("Sequential packets - no loss", "[seq_tracker]") {
    SeqTracker t;
    feed(t, {0, 1, 2, 3, 4});

    REQUIRE(t.stats.rx      == 5);
    REQUIRE(t.stats.missing == 0);
    REQUIRE(t.stats.ooo     == 0);
    REQUIRE(t.stats.dup     == 0);
}

TEST_CASE("Single missing packet", "[seq_tracker]") {
    SeqTracker t;
    feed(t, {0, 1, 3}); // seq=2 kayip

    REQUIRE(t.stats.rx      == 3);
    REQUIRE(t.stats.missing == 1);
    REQUIRE(t.stats.ooo     == 0);
}

TEST_CASE("Multiple missing packets", "[seq_tracker]") {
    SeqTracker t;
    feed(t, {0, 5}); // 1,2,3,4 kayip

    REQUIRE(t.stats.missing == 4);
}

TEST_CASE("Duplicate packet", "[seq_tracker]") {
    SeqTracker t;
    feed(t, {0, 1, 1, 2});

    REQUIRE(t.stats.dup     == 1);
    REQUIRE(t.stats.missing == 0);
    REQUIRE(t.stats.ooo     == 0);
}

TEST_CASE("Out of order packet", "[seq_tracker]") {
    SeqTracker t;
    feed(t, {0, 1, 2, 1, 3});

    REQUIRE(t.stats.ooo >= 1);
}

TEST_CASE("Wrap-around: 0xFFFFFFFF -> 0x00000000 is valid next packet", "[seq_tracker][wrap]") {
    SeqTracker t;
    t.update(0xFFFFFFFE);
    t.update(0xFFFFFFFF);
    t.update(0x00000000);

    REQUIRE(t.stats.missing == 0);
    REQUIRE(t.stats.ooo     == 0);
    REQUIRE(t.stats.rx      == 3);
}

TEST_CASE("Wrap-around: missing packet across boundary", "[seq_tracker][wrap]") {
    SeqTracker t;
    t.update(0xFFFFFFFE);
    t.update(0x00000001); // 0xFFFFFFFF ve 0x00000000 kayip -> 2 kayip

    REQUIRE(t.stats.missing == 2);
    REQUIRE(t.stats.ooo     == 0);
}

TEST_CASE("Wrap-around: last stays correct after wrap", "[seq_tracker][wrap]") {
    SeqTracker t;
    t.update(0xFFFFFFFF);
    t.update(0x00000000);
    t.update(0x00000001);

    REQUIRE(t.stats.missing == 0);
    REQUIRE(t.stats.ooo     == 0);
    REQUIRE(t.last          == 0x00000001);
}

TEST_CASE("PLR calculation", "[seq_tracker]") {
    SeqTracker t;
    // 0,1 geldi, 2 ve 3 kayip, 4 geldi -> missing=2, rx=3
    // plr = 2 / (3 + 2) = 2/5 = 0.4
    feed(t, {0, 1, 4});

    REQUIRE_THAT(t.plr(), Catch::Matchers::WithinAbs(2.0 / 5.0, 1e-9));
}

TEST_CASE("PLR is zero when no loss", "[seq_tracker]") {
    SeqTracker t;
    feed(t, {0, 1, 2, 3});

    REQUIRE(t.plr() == 0.0);
}

TEST_CASE("PLR on empty tracker is zero", "[seq_tracker]") {
    SeqTracker t;
    REQUIRE(t.plr() == 0.0);
}

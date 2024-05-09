#include "sese/util/DateTime.h"
#include "sese/record/LogHelper.h"
#include "sese/record/Marco.h"
#include "sese/text/DateTimeFormatter.h"
#include "gtest/gtest.h"

#include <thread>

using namespace std::chrono_literals;

TEST(TestDateTime, Info) {
    sese::record::LogHelper log;
    auto time = sese::DateTime::nowPtr(8);
    log.info("is leap year: %s", time->isLeapYear() ? "True" : "False");
    log.info("timestamp: %lld", time->getTimestamp());
    log.info(
            "date: %d-%d-%d %d:%d:%d",
            time->getYears(),
            time->getMonths(),
            time->getDays(),
            time->getHours(),
            time->getMinutes(),
            time->getSeconds()
    );
    log.info("day of year: %d", time->getDayOfYear());
    log.info("day of week: %d", time->getDayOfWeek());
}

TEST(TestDateTime, InfoCheck) {
    auto tm = sese::DateTime(1703758865'114'514, 0);
    EXPECT_EQ(tm.getYears(), 2023);
    EXPECT_EQ(tm.getMonths(), 12);
    EXPECT_EQ(tm.getDays(), 28);
    EXPECT_EQ(tm.getHours(), 10);
    EXPECT_EQ(tm.getMinutes(), 21);
    EXPECT_EQ(tm.getSeconds(), 05);
    EXPECT_EQ(tm.getMilliseconds(), 114);
    EXPECT_EQ(tm.getMicroseconds(), 514);
    EXPECT_EQ(tm.getUTC(), 0);
}

TEST(TestDateTime, Compare) {
    // 2022-03-03 00:00:00
    auto time1 = sese::DateTime(1646265600000000, 0);
    // 2022-03-01 00:00:00
    auto time2 = sese::DateTime(1646092800000000, 0);

    EXPECT_EQ(time1.compareTo(time2), 1);
    EXPECT_EQ(time2.compareTo(time1), -1);
}

TEST(TestDateTime, UnclearCompare) {
    {
        auto time1 = sese::DateTime::now(8);
        auto time2 = sese::DateTime::now(8);
        EXPECT_EQ(time1.unclearCompareTo(time2), 0);
    }

    {
        auto time1 = sese::DateTime::now(8);
        // std::this_thread::sleep_for(1s);
        auto time2 = sese::DateTime(time1.getTimestamp() + 1002'333);
        EXPECT_EQ(time1.unclearCompareTo(time2), -1);
        EXPECT_EQ(time2.unclearCompareTo(time1), 1);
    }
}

TEST(TestDateTime, Operator) {
    // 2022-03-03 00:00:00 UTC+8
    auto time1 = sese::DateTime(1646236800000000, 0);
    // 2022-03-01 00:00:00 UTC+8
    auto time2 = sese::DateTime(1646064000000000, 0);

    auto span1 = time1 - time2;
    EXPECT_EQ(span1.getDays(), 2);

    // 2022-03-05 00:00:00 UTC+8
    auto time3 = sese::DateTime(1646409600000000, 0);
    EXPECT_EQ(time3.compareTo(time1 + span1), 0);

    EXPECT_EQ(time2.compareTo(time1 - span1), 0);
}

TEST(TestDateTime, Leap) {
    auto time1 = sese::DateTime(1072915200000000, 0);
    EXPECT_EQ(time1.isLeapYear(), true);

    auto time2 = sese::DateTime(946684800000000, 0);
    EXPECT_EQ(time2.isLeapYear(), true);

    auto time3 = sese::DateTime(978307200000000, 0);
    EXPECT_EQ(time3.isLeapYear(), false);
}

TEST(TestDateTime, Parse) {
    using sese::text::DateTimeFormatter;

    EXPECT_EQ(
            DateTimeFormatter::parseFromGreenwich("Tue, 17 Oct 2023 15:41:22 GMT"),
            1697557282
    );

    EXPECT_EQ(
            DateTimeFormatter::parseFromISO8601("2023-06-20"),
            1687219200
    );

    EXPECT_EQ(
            DateTimeFormatter::parseFromISO8601("2023-06-20 16:46:55"),
            1687279615
    );

    EXPECT_EQ(
            DateTimeFormatter::parseFromISO8601("2023-06-20T16:46:55Z"),
            1687279615
    );

    EXPECT_EQ(
            DateTimeFormatter::parseFromISO8601("2023-06-20 16:46:55 +8"),
            1687250815
    );

    EXPECT_EQ(
            DateTimeFormatter::parseFromISO8601("2023-06-20T16:46:55+00:00"),
            1687279615
    );
}

TEST(TestDateTime, Format_0) {
    auto time = sese::DateTime(978307261001001, 0);

    auto str1 = sese::text::DateTimeFormatter::format(time, "y-M-d h:m:s.f.fff t z");
    sese::record::LogHelper::i(str1.c_str());

    auto str2 = sese::text::DateTimeFormatter::format(time, "yy-MM-dd hh:mm:ss.ff.ffff t z");
    sese::record::LogHelper::i(str2.c_str());

    auto str3 = sese::text::DateTimeFormatter::format(time, "yyy-MMM-dd ddd H:mm:s");
    sese::record::LogHelper::i(str3.c_str());

    auto str4 = sese::text::DateTimeFormatter::format(time, "yyyy-MMMM-dd dddd HH:mm:ss");
    sese::record::LogHelper::i(str4.c_str());
}

TEST(TestDateTime, Format_1) {
    auto time = sese::DateTime(61653571010010, -1);

    auto str1 = sese::text::DateTimeFormatter::format(time, "y-M-d h:m:s.f.fff t z");
    sese::record::LogHelper::i(str1.c_str());

    auto str2 = sese::text::DateTimeFormatter::format(time, "yy-MM-dd hh:mm:ss.ff.ffff t z");
    sese::record::LogHelper::i(str2.c_str());

    auto str3 = sese::text::DateTimeFormatter::format(time, "yyy-MMM-dd ddd H:mm:s");
    sese::record::LogHelper::i(str3.c_str());

    auto str4 = sese::text::DateTimeFormatter::format(time, "yyyy-MMMM-dd dddd HH:mm:ss");
    sese::record::LogHelper::i(str4.c_str());
}

TEST(TestDateTime, Format_2) {
    auto time = sese::DateTime(61613971114514, 0);

    auto str1 = sese::text::DateTimeFormatter::format(time, "y-M-d h:m:s.f.fff t z");
    sese::record::LogHelper::i(str1.c_str());

    auto str2 = sese::text::DateTimeFormatter::format(time, "yy-MM-dd hh:mm:ss.ff.ffff t z");
    sese::record::LogHelper::i(str2.c_str());

    auto str3 = sese::text::DateTimeFormatter::format(time, "yyy-MMM-dd ddd H:mm:s");
    sese::record::LogHelper::i(str3.c_str());

    auto str4 = sese::text::DateTimeFormatter::format(time, "yyyy-MMMM-dd dddd HH:mm:ss %");
    sese::record::LogHelper::i(str4.c_str());
}

TEST(TestDateTime, Format_3) {
    auto time = sese::DateTime::nowPtr(0);

    auto str1 = sese::text::DateTimeFormatter::format(time, "y-M-d h:m:s.f.fff t z");
    sese::record::LogHelper::i(str1.c_str());

    auto str2 = sese::text::DateTimeFormatter::format(time, "yy-MM-dd hhh:mmm:sss.ff.ffff t z");
    sese::record::LogHelper::i(str2.c_str());

    auto str3 = sese::text::DateTimeFormatter::format(time, "yyy-MMM-dd ddd H:mm:s");
    sese::record::LogHelper::i(str3.c_str());

    auto str4 = sese::text::DateTimeFormatter::format(time, "yyyy-MMMM-dd dddd HH:mm:ss %");
    sese::record::LogHelper::i(str4.c_str());
}

TEST(TestTimeSpan, Total) {
    auto time = sese::TimeSpan(1, 16, 30, 5, 100, 300);
    SESE_INFO("total days %f", time.getTotalDays());
    SESE_INFO("total hours %f", time.getTotalHours());
    SESE_INFO("total minutes %f", time.getTotalMinutes());
    SESE_INFO("total seconds %f", time.getTotalSeconds());
    SESE_INFO("total milliseconds %f", time.getTotalMilliseconds());
    SESE_INFO("total microseconds %" PRIu64, time.getTotalMicroseconds());
}

TEST(TestTimeSpan, Part) {
    auto time = sese::TimeSpan(1, 25, 30, 5, 10010, 3005);
    SESE_INFO("days %" PRId32, time.getDays());
    SESE_INFO("hours %" PRId32, time.getHours());
    SESE_INFO("minutes %" PRId32, time.getMinutes());
    SESE_INFO("seconds %" PRId32, time.getSeconds());
    SESE_INFO("milliseconds %" PRId32, time.getMilliseconds());
    SESE_INFO("microseconds %" PRId32, time.getMicroseconds());
}

#include <sese/text/DateTimeParser.h>

using sese::DateTime;
using sese::text::DateTimeParser;

TEST(TestDateTime, Parser_1) {
    auto tm = DateTimeParser::parse("yyyy-MM-dd HH:mm:ss", "2020-10-07 23:29:40");
    ASSERT_TRUE(tm.has_value());
    EXPECT_EQ(1602113380'000'000, tm.value().getTimestamp());
}

TEST(TestDateTime, Parser_2) {
    auto tm = DateTimeParser::parse("yyyy-MM-dd HH:mm:ss * z", "2020-01-03 19:23:51 GTM +08");
    ASSERT_TRUE(tm.has_value());
    EXPECT_EQ(1578050631'000'000, tm.value().getTimestamp());
}

TEST(TestDateTime, Parser_3) {
    auto tm = DateTimeParser::parse("yyyy-MM-dd HH:mm:ss fff rrr * z", "2020-01-03 19:23:51 985 211 GTM +08");
    ASSERT_TRUE(tm.has_value());
    EXPECT_EQ(1578050631'985'211, tm.value().getTimestamp());
}

TEST(TestDateTime, Parse_Empty) {
    EXPECT_FALSE(DateTimeParser::parse("", "").has_value());
    EXPECT_FALSE(DateTimeParser::parse("content", "").has_value());
    EXPECT_FALSE(DateTimeParser::parse("", "content").has_value());
}
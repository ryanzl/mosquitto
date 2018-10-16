#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include <util_mosq.h>

static void match_helper(const char *sub, const char *topic)
{
	int rc;
	bool match;

	rc = mosquitto_topic_matches_sub(sub, topic, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_SUCCESS);
	CU_ASSERT_EQUAL(match, true);
}

static void no_match_helper(int rc_expected, const char *sub, const char *topic)
{
	int rc;
	bool match;

	rc = mosquitto_topic_matches_sub(sub, topic, &match);
	CU_ASSERT_EQUAL(rc, rc_expected);
	CU_ASSERT_EQUAL(match, false);
}

/* ========================================================================
 * EMPTY INPUT
 * ======================================================================== */

static void TEST_empty_input(void)
{
	int rc;
	bool match;

	rc = mosquitto_topic_matches_sub("sub", NULL, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub(NULL, "topic", &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub(NULL, NULL, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub("sub", "", &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub("", "topic", &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub("", "", &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub2("sub", 3, NULL, 0, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub2(NULL, 0, "topic", 5, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub2(NULL, 0, NULL, 0, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub2("sub", 3, "", 0, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub2("", 0, "topic", 5, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);

	rc = mosquitto_topic_matches_sub2("", 0, "", 0, &match);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
	CU_ASSERT_EQUAL(match, false);
}

/* ========================================================================
 * VALID MATCHING AND NON-MATCHING
 * ======================================================================== */

static void TEST_valid_matching(void)
{
	match_helper("foo/#", "foo/");
	match_helper("foo/#", "foo");
	match_helper("foo//bar", "foo//bar");
	match_helper("foo//+", "foo//bar");
	match_helper("foo/+/+/baz", "foo///baz");
	match_helper("foo/bar/+", "foo/bar/");
	match_helper("foo/bar", "foo/bar");
	match_helper("foo/+", "foo/bar");
	match_helper("foo/+/baz", "foo/bar/baz");
	match_helper("A/B/+/#", "A/B/B/C");
	match_helper("foo/+/#", "foo/bar/baz");
	match_helper("foo/+/#", "foo/bar");
	match_helper("#", "foo/bar/baz");
	match_helper("#", "foo/bar/baz");
	match_helper("#", "/foo/bar");
	match_helper("/#", "/foo/bar");
}


static void TEST_invalid_but_matching(void)
{
	/* Matching here is "naive treatment of the wildcards would produce a
	 * match". They shouldn't really match, they should fail. */
	no_match_helper(MOSQ_ERR_INVAL, "+foo", "+foo");
	no_match_helper(MOSQ_ERR_INVAL, "fo+o", "fo+o");
	no_match_helper(MOSQ_ERR_INVAL, "foo+", "foo+");
	no_match_helper(MOSQ_ERR_INVAL, "+foo/bar", "+foo/bar");
	no_match_helper(MOSQ_ERR_INVAL, "foo+/bar", "foo+/bar");
	no_match_helper(MOSQ_ERR_INVAL, "foo/+bar", "foo/+bar");
	no_match_helper(MOSQ_ERR_INVAL, "foo/bar+", "foo/bar+");

	no_match_helper(MOSQ_ERR_INVAL, "+foo", "afoo");
	no_match_helper(MOSQ_ERR_INVAL, "fo+o", "foao");
	no_match_helper(MOSQ_ERR_INVAL, "foo+", "fooa");
	no_match_helper(MOSQ_ERR_INVAL, "+foo/bar", "afoo/bar");
	no_match_helper(MOSQ_ERR_INVAL, "foo+/bar", "fooa/bar");
	no_match_helper(MOSQ_ERR_INVAL, "foo/+bar", "foo/abar");
	no_match_helper(MOSQ_ERR_INVAL, "foo/bar+", "foo/bara");

	no_match_helper(MOSQ_ERR_INVAL, "#foo", "#foo");
	no_match_helper(MOSQ_ERR_INVAL, "fo#o", "fo#o");
	no_match_helper(MOSQ_ERR_INVAL, "foo#", "foo#");
	no_match_helper(MOSQ_ERR_INVAL, "#foo/bar", "#foo/bar");
	no_match_helper(MOSQ_ERR_INVAL, "foo#/bar", "foo#/bar");
	no_match_helper(MOSQ_ERR_INVAL, "foo/#bar", "foo/#bar");
	no_match_helper(MOSQ_ERR_INVAL, "foo/bar#", "foo/bar#");

	no_match_helper(MOSQ_ERR_INVAL, "foo+", "fooa");
}


static void TEST_valid_no_matching(void)
{
	no_match_helper(MOSQ_ERR_SUCCESS, "test/6/#", "test/3");

	no_match_helper(MOSQ_ERR_SUCCESS, "foo/bar", "foo");
	no_match_helper(MOSQ_ERR_SUCCESS, "foo/+", "foo/bar/baz");
	no_match_helper(MOSQ_ERR_SUCCESS, "foo/+/baz", "foo/bar/bar");

	no_match_helper(MOSQ_ERR_SUCCESS, "foo/+/#", "fo2/bar/baz");

	no_match_helper(MOSQ_ERR_SUCCESS, "/#", "foo/bar");
	no_match_helper(MOSQ_ERR_SUCCESS, "/#a", "foo/bar");

	no_match_helper(MOSQ_ERR_SUCCESS, "#", "$SYS/bar");
	no_match_helper(MOSQ_ERR_SUCCESS, "$BOB/bar", "$SYS/bar");
}


static void TEST_invalid(void)
{
	no_match_helper(MOSQ_ERR_INVAL, "foo#", "foo");
	no_match_helper(MOSQ_ERR_INVAL, "fo#o/", "foo");
	no_match_helper(MOSQ_ERR_INVAL, "foo#", "fooa");
	no_match_helper(MOSQ_ERR_INVAL, "foo+", "foo");
}

/* ========================================================================
 * TEST SUITE SETUP
 * ======================================================================== */

int init_util_topic_tests(void)
{
	CU_pSuite test_suite = NULL;

	test_suite = CU_add_suite("Util topic", NULL, NULL);
	if(!test_suite){
		printf("Error adding CUnit util topic test suite.\n");
		return 1;
	}

	if(0
			|| !CU_add_test(test_suite, "Empty input", TEST_empty_input)
			|| !CU_add_test(test_suite, "Valid matching", TEST_valid_matching)
			|| !CU_add_test(test_suite, "Valid no matching", TEST_valid_no_matching)
			|| !CU_add_test(test_suite, "Invalid but matching", TEST_invalid_but_matching)
			|| !CU_add_test(test_suite, "Invalid", TEST_invalid)
			){

		printf("Error adding util topic CUnit tests.\n");
		return 1;
	}

	return 0;
}

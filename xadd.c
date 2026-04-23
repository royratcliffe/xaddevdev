#include "names.h"
#include "pr.h"
#include "when.h"

#include <hiredis.h>
#include <linux/input.h>
#include <stdarg.h>

static redisOptions options = {
    .type = REDIS_CONN_TCP,
    .endpoint.tcp.ip = "localhost",
    .endpoint.tcp.port = 6379,
};

static redisContext *redis;

CAUSES(opt_h, xadd_opt_h) {
  const char *optarg = with;
  pr_info("Redis host: %s\n", optarg);
  options.endpoint.tcp.ip = optarg;
}

CAUSES(opt_p, xadd_opt_p) {
  const char *optarg = with;
  pr_info("Redis port: %s\n", optarg);
  options.endpoint.tcp.port = atoi(optarg);
}

CAUSES(epoll, xadd_epoll) {
  pr_debug("Redis epoll event handler registered\n");
  redis = redisConnectWithOptions(&options);
  if (redis == NULL || redis->err) {
    pr_err("Failed to connect to Redis: %s\n", redis ? redis->errstr : "unknown error");
    exit(EXIT_FAILURE);
  }
}

/*!
 * \brief Converts a timeval structure to milliseconds.
 * \param tv A pointer to a timeval structure containing the time value to convert.
 * \return The time value in milliseconds as a long long integer.
 * \details This function takes a pointer to a timeval structure, which
 * contains a time value in seconds and microseconds, and converts it to
 * milliseconds. The conversion is done by multiplying the seconds by 1000 to
 * convert them to milliseconds, and then adding the microseconds divided by
 * 1000 to convert them to milliseconds as well. The resulting value is
 * returned as a long long integer, which can represent a wide range of time
 * values in milliseconds. This function is useful for converting time values
 * to a common unit (milliseconds) that can be easily used for timing and
 * scheduling purposes, such as when adding entries to a Redis stream with a
 * timestamp or when calculating time differences for event handling.
 * \note It rounds down the microseconds to the nearest millisecond, which may
 * result in a loss of precision for time values that are not an exact multiple
 * of 1000 microseconds. However, this is generally acceptable for most use
 * cases where millisecond precision is sufficient, and it simplifies the
 * conversion process by avoiding the need for floating-point arithmetic or more
 * complex rounding logic. If higher precision is required, additional logic can
 * be implemented to handle the rounding of microseconds to the nearest
 * millisecond more accurately.
 */
static inline long long timeval_ms(struct timeval *tv) { return tv->tv_sec * 1000LL + tv->tv_usec / 1000LL; }

CAUSES(input_event, xadd_input_event) {
  if (redis == NULL) {
    return;
  }

  struct input_event *input_event = (struct input_event *)with;
  va_list args;
  va_start(args, with);
  const char *name = va_arg(args, const char *);
  va_end(args);

  /*
   * Use XADD with the MINID option to add the input event to the stream. The
   * MINID option allows us to specify a minimum ID for the new entry, which can
   * be used to ensure that the new entry is added with an ID that is greater
   * than a certain value. In this case, we calculate the minimum ID based on
   * the current time minus a certain duration (e.g., 10 seconds) to ensure that
   * the new entry is added with an ID that is greater than the minimum ID,
   * which can help to prevent issues with clock skew or other timing-related
   * issues that could arise when adding entries to the stream. By using the
   * MINID option, we can ensure that the new entry is added with a valid ID
   * that is greater than the specified minimum ID, which can help to maintain
   * the integrity of the stream and ensure that entries are added in a
   * consistent and predictable manner. Additionally, we can include the device
   * name as a field in the stream entry, along with the timestamp of the input
   * event and the type, code, and value of the input event, to provide more
   * context and information about the event that occurred. This can be useful
   * for debugging and analysis purposes, as it allows us to track the input
   * events and their associated metadata in the Redis stream, and can help us
   * to identify patterns or trends in the input events over time. By including
   * the device name and other relevant information in the stream entry, we can
   * gain insights into the input events and their characteristics, which can be
   * valuable for understanding user interactions and improving the overall user
   * experience.
   */
  struct timeval tv;
  gettimeofday(&tv, NULL);
  const long long min_id = timeval_ms(&tv) - (10 * 1000LL);
  const long long time = timeval_ms(&input_event->time);

  redisReply *reply =
      redisCommand(redis, "XADD input_event MINID ~ %lld * device \"%s\" time %lld type %u typename %s code %u codename %s value %d", min_id, name, time,
                   input_event->type, typename(input_event->type), input_event->code, codename(input_event->type, input_event->code), input_event->value);
  if (reply == NULL) {
    pr_err("Failed to add input event to Redis: %s\n", redis->errstr);
    return;
  }
  switch (reply->type) {
  case REDIS_REPLY_STRING:
    pr_debug("Redis reply: %s\n", reply->str);
    break;
  case REDIS_REPLY_ARRAY:
    pr_debug("Redis reply is an array with %zu elements.\n", reply->elements);
    break;
  case REDIS_REPLY_ERROR:
    pr_warn("Error reply from Redis: %s\n", reply->str);
    break;
  default:
    pr_info("Redis reply type: %d\n", reply->type);
    break;
  }
  freeReplyObject(reply);
  pr_info("Input event: device=%s time=%lld type=%u code=%u value=%d\n", name, time, input_event->type, input_event->code, input_event->value);
}

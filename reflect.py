import math

origin = [20, 20]
direction = math.pi / 4
length = 500

mirrors = [[25, 0, 25, 10]]
max_reflections = 100


def next_segment():
    return [
        origin[0],
        origin[1],
        origin[0] + length * math.cos(direction),
        origin[1] + length * math.sin(direction),
    ]


def solve2(a1, b1, c1, a2, b2, c2):
    den = a1 * b2 - b1 * a2
    if math.isclose(den, 0):
        return (None, None)
    return ((c1 * b2 - b1 * c2) / den, (a1 * c2 - c1 * a2) / den)


def intersection_point(x1, y1, x2, y2, x3, y3, x4, y4):
    s0, t0 = solve2(x2 - x1, -(x4 - x3), x3 - x1, y2 - y1, -(y4 - y3), y3 - y1)
    if not s0 or not t0 or s0 < 0 or s0 > 1 or t0 < 0 or t0 > 1:
        return None
    p = x1 + s0 * (x2 - x1)
    q = y1 + s0 * (y2 - y1)
    return (p, q)


def line_angle(x1, y1, x2, y2):
    return math.atan2(y2 - y1, x2 - x1)


def distance(x1, y1, x2, y2):
    return math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))


segments = [next_segment()]
n = 0
mirror_last = -1
while True:
    reflection_point = None
    mirror_angle = 0
    mirror_dist = 0
    mirror_idx = -1
    for i in range(len(mirrors)):
        if i == mirror_last:
            # print("already hit", i, mirrors[i])
            continue

        point = intersection_point(
            segments[n][0],
            segments[n][1],
            segments[n][2],
            segments[n][3],
            mirrors[i][0],
            mirrors[i][1],
            mirrors[i][2],
            mirrors[i][3],
        )

        if not point:
            continue

        # print("hit mirror", i, "at", point)

        # find the closest mirror the ray intersects with
        if not reflection_point:
            reflection_point = point
            mirror_angle = line_angle(
                mirrors[i][0], mirrors[i][1], mirrors[i][2], mirrors[i][3]
            )
            mirror_dist = distance(segments[n][0], segments[n][1], point[0], point[1])
            mirror_idx = i
        else:
            dist = distance(segments[n][0], segments[n][1], point[0], point[1])
            if dist < mirror_dist:
                reflection_point = point
                mirror_angle = line_angle(
                    mirrors[i][0], mirrors[i][1], mirrors[i][2], mirrors[i][3]
                )
                mirror_dist = dist
                mirror_idx = i

    if not reflection_point:
        # doesn't intersect with any, the end
        break

    mirror_last = mirror_idx

    segment_angle = line_angle(
        segments[n][0], segments[n][1], segments[n][2], segments[n][3]
    )
    direction = 2 * (mirror_angle - segment_angle) + segment_angle
    segments[n][2] = reflection_point[0]
    segments[n][3] = reflection_point[1]
    origin = reflection_point
    length = length - mirror_dist
    segments += [next_segment()]
    n += 1
    if n >= max_reflections:
        print("reflection limit reached")
        break

print("ray segments:")
for s in segments:
    print(
        f"({s[0]:.2f}, {s[1]:.2f}) -> ({s[2]:.2f}, {s[3]:.2f}) at {math.degrees(line_angle(*s)):.1f}°"
    )

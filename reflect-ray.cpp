#include <SDL.h>
#include <cmath>
#include <vector>

using namespace std;

#define EPS 1e-6

SDL_Window* window;
SDL_Renderer* renderer;
const int FPS = 60;
SDL_Rect world = {0, 0, 300, 200};
float reflection_distance = 1;

struct Segment
{
	float x1, y1, x2, y2;
};

vector<Segment> mirrors =
{
	{(float)world.x, (float)world.y, (float)world.w-1, (float)world.y},
	{(float)world.w-1, (float)world.y, (float)world.w-1, (float)world.h-1},
	{(float)world.x, (float)world.h-1, (float)world.w-1, (float)world.h-1},
	{(float)world.x, (float)world.y, (float)world.x, (float)world.h-1},
	{0, 100, 100, 100},
};

float distance(SDL_FPoint p, SDL_FPoint q)
{
	return sqrtf((q.x - p.x) * (q.x - p.x) + (q.y - p.y) * (q.y - p.y));
}

float line_angle(Segment line)
{
	return atan2f(line.y2 - line.y1, line.x2 - line.x1);
}

bool solve2(float a1, float b1, float c1, float a2, float b2, float c2,
            float& s, float& t)
{
	float den = a1 * b2 - b1 * a2;
	if (fabs(den) < EPS)
		return false;
	s = (c1 * b2 - b1 * c2) / den;
	t = (a1 * c2 - c1 * a2) / den;
	return true;
}

bool intersection_point(Segment seg1, Segment seg2, SDL_FPoint& p)
{
	float s0, t0;
	bool result = solve2(seg1.x2 - seg1.x1, seg2.x1 - seg2.x2, seg2.x1 - seg1.x1,
	                     seg1.y2 - seg1.y1, seg2.y1 - seg2.y2, seg2.y1 - seg1.y1,
	                     s0, t0);
	if (!result || s0 < 0.0f || s0 > 1.0f || t0 < 0.0f || t0 > 1.0f)
	{
		return false;
	}
	p.x = seg1.x1 + s0 * (seg1.x2 - seg1.x1);
	p.y = seg1.y1 + s0 * (seg1.y2 - seg1.y1);
	return true;
}

class Trajectory
{
	SDL_FPoint origin;
	float length;
	float angle;
	vector<SDL_FPoint> points;

public:
	Trajectory(SDL_FPoint origin, float length, float angle) : origin(origin), length(length), angle(angle) {}

	void draw()
	{
		calc_points();
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		for (int i = 1; i < points.size(); ++i)
		{
			SDL_FPoint segment_start = {points[i-1].x, points[i-1].y};
			SDL_FPoint segment_end = {points[i].x, points[i].y};
			SDL_RenderDrawLine(renderer,
			                   segment_start.x, segment_start.y,
			                   segment_end.x, segment_end.y);
		}
	}

private:
	void calc_points()
	{
		points.clear();
		points.push_back({origin.x, origin.y});
		float remaining_length = length;
		int n = 0;
		float direction = angle;
		int mirror_last = -1;
		while (true)
		{
			Segment ray = {points[n].x, points[n].y,
			               points[n].x + remaining_length * cosf(direction),
			               points[n].y + remaining_length * sinf(direction)};
			points.push_back({ray.x2, ray.y2});
			++n;
			int mirror_idx = -1;
			float mirror_dist;
			SDL_FPoint reflection_point;
			for (int i = 0; i < mirrors.size(); ++i)
			{
				if (i == mirror_last)
					continue;
				SDL_FPoint point;
				bool result = intersection_point(ray, mirrors[i], point);
				if (!result)
				{
					continue;
				}
				float dist = distance({ray.x1, ray.y1}, point);
				if (mirror_idx < 0 || dist < mirror_dist)
				{
					reflection_point = point;
					mirror_idx = i;
					mirror_dist = dist;
				}
			}
			if (mirror_idx < 0)
			{
				break;
			}
			mirror_last = mirror_idx;
			float ray_angle = line_angle(ray);
			float mirror_angle = line_angle(mirrors[mirror_last]);
			mirror_dist -= reflection_distance;
			reflection_point = {ray.x1 + mirror_dist * cosf(ray_angle), ray.y1 + mirror_dist * sinf(ray_angle)};
			points[n] = reflection_point;
			ray.x2 = reflection_point.x;
			ray.y2 = reflection_point.y;
			direction = 2 * (mirror_angle - ray_angle) + ray_angle;
			remaining_length -= mirror_dist;
		}
	}
};

class Player
{
	SDL_FPoint pos;
	float angle;
	bool firing;
	float shooting_range;

public:
	Player(SDL_FPoint pos, float angle, float shooting_range) : pos(pos), angle(angle), shooting_range(shooting_range) {}

	void turn_mouse(float logical_mouse_x, float logical_mouse_y)
	{
		angle = atan2f(logical_mouse_y - pos.y, logical_mouse_x - pos.x);
	}

	void set_firing(float firing)
	{
		this->firing = firing;
	}

	void move(bool forward, bool backward, bool left, bool right)
	{
		if (forward) pos.y -= 1;
		if (backward) pos.y += 1;
		if (left) pos.x -= 1;
		if (right) pos.x += 1;
	}

	void draw()
	{
		if (firing)
		{
			Trajectory tr(pos, shooting_range, angle);
			tr.draw();
		}
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_Rect rect = {(int)pos.x, (int)pos.y, 1, 1};
		SDL_RenderFillRect(renderer, &rect);
	}
};

void delay_frame(Uint64& cnt_start, Uint64& cnt_end)
{
	float cnt_delta = (float)(cnt_end - cnt_start)/SDL_GetPerformanceFrequency();
	if (cnt_delta < 1.0f/FPS)
	{
		SDL_Delay((1.0f/FPS - cnt_delta)*999);
		cnt_end = SDL_GetPerformanceCounter();
		cnt_delta = (float)(cnt_end - cnt_start)/SDL_GetPerformanceFrequency();
	}
	while (cnt_delta < 1.0f/FPS)
	{
		cnt_end = SDL_GetPerformanceCounter();
		cnt_delta = (float)(cnt_end - cnt_start)/SDL_GetPerformanceFrequency();
	}
}

int main(int argc, char** argv)
{
	Player player({world.w/2.0f, world.h/2.0f}, -M_PI/4.0f, 1000.0f);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(900, 600, SDL_WINDOW_RESIZABLE, &window, &renderer);
	SDL_RenderSetLogicalSize(renderer, world.w, world.h);

	Uint64 cnt_end = SDL_GetPerformanceCounter();
	while (1)
	{
		Uint64 cnt_start = cnt_end;

		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				SDL_DestroyWindow(window);
				SDL_DestroyRenderer(renderer);
				SDL_Quit();
				exit(0);
			}
		}

		int mouse_x, mouse_y;
		Uint32 buttons;
		buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
		player.set_firing(buttons & SDL_BUTTON_LMASK);

		float logical_mouse_x;
		float logical_mouse_y;
		SDL_RenderWindowToLogical(renderer, mouse_x, mouse_y, &logical_mouse_x, &logical_mouse_y);
		player.turn_mouse(logical_mouse_x, logical_mouse_y);

		const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
		player.move(keyboard_state[SDL_SCANCODE_W],
		            keyboard_state[SDL_SCANCODE_S],
		            keyboard_state[SDL_SCANCODE_A],
		            keyboard_state[SDL_SCANCODE_D]);

		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		SDL_RenderFillRect(renderer, &world);		
		player.draw();
		for (int i = 0; i < mirrors.size(); ++i)
		{
			SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
			SDL_RenderDrawLine(renderer, mirrors[i].x1, mirrors[i].y1, mirrors[i].x2, mirrors[i].y2);
		}
		SDL_RenderPresent(renderer);
		cnt_end = SDL_GetPerformanceCounter();
		delay_frame(cnt_start, cnt_end);
	}
}
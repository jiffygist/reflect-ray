#include <SDL.h>
#include <cmath>
#include <vector>

using namespace std;

#define EPS 1e-6

SDL_Window* window;
SDL_Renderer* renderer;
const int FPS = 60;
SDL_Rect world = {0, 0, 60, 80};

struct Segment
{
	float x1, y1, x2, y2;
};

vector<Segment> mirrors = 
{
	// {(float)world.x, (float)world.y, (float)world.w-1, (float)world.y},
	// {(float)world.w-1, (float)world.y, (float)world.w-1, (float)world.h-1},
	// {(float)world.x, (float)world.h-1, (float)world.w-1, (float)world.h-1},
	// {(float)world.x, (float)world.y, (float)world.x, (float)world.h-1},
	// {0, 30, 30, 30},
	{25, 0, 25, 10}
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
	                     seg1.y2 - seg1.y1, seg2.y1 - seg2.y2, seg2.y2 - seg1.y1,
	                     s0, t0);
	if (!result || s0 < 0.0f || s0 > 1.0f || t0 < 0.0f || t0 > 1.0f)
		return false;
	p.x = seg1.x1 + s0 * (seg1.x2 - seg1.x1);
	p.y = seg1.y1 + s0 * (seg1.y2 - seg1.y1);
	return true;
}

struct Trajectory
{
	SDL_FPoint origin;
	float length;
	float angle;

	vector<SDL_FPoint> points;

	void draw()
	{
		calc_points();
		
		for (int i = 1; i < points.size(); ++i)
		{
			SDL_FPoint segment_start = {points[i-1].x, points[i-1].y};
			SDL_FPoint segment_end = {points[i].x, points[i].y};
			SDL_RenderDrawLine(renderer,
			                   segment_start.x, segment_start.y,
			                   segment_end.x, segment_end.y);
			segment_start = segment_end;
		}
	}

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
				SDL_Log("no intersects");
				break;
			}
			mirror_last = mirror_idx;
			SDL_Log("reflection off mirror %d at %lf, %lf", mirror_last, reflection_point.x, reflection_point.y);
			points[n] = reflection_point;
			ray.x2 = reflection_point.x;
			ray.y2 = reflection_point.y;
			float ray_angle = line_angle(ray);
			float mirror_angle = line_angle(mirrors[mirror_last]);
			direction = 2 * (mirror_angle - ray_angle) + ray_angle;
			remaining_length -= mirror_dist;
		}
		SDL_Log("%ld\n", points.size());
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

int main()
{
	Trajectory tr = {{20.0f, 20.0f}, 100.0f, -M_PI/4.0f};

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(600, 800, SDL_WINDOW_RESIZABLE, &window, &renderer);
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
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		SDL_RenderFillRect(renderer, &world);
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		tr.draw();
		// tr.angle += 0.01;
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
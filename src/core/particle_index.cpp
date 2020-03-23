#include "particle_index.hpp"

std::vector<Particle *> local_particles;

void update_local_particles(ParticleList *pl) {
  Particle *p = pl->part;
  int n = pl->n, i;
  for (i = 0; i < n; i++)
    set_local_particle_data(p[i].p.identity, &p[i]);
}

void append_unindexed_particle(ParticleList *l, Particle &&part) {
  l->resize(l->n + 1);
  new (&(l->part[l->n - 1])) Particle(std::move(part));
}

Particle *append_indexed_particle(ParticleList *l, Particle &&part) {
  auto const re = l->resize(l->n + 1);
  auto p = new (&(l->part[l->n - 1])) Particle(std::move(part));

  if (re)
    update_local_particles(l);
  else
    set_local_particle_data(p->p.identity, p);
  return p;
}

Particle *move_unindexed_particle(ParticleList *dl, ParticleList *sl, int i) {
  assert(sl->n > 0);
  assert(i < sl->n);

  dl->resize(dl->n + 1);
  auto dst = &dl->part[dl->n - 1];
  auto src = &sl->part[i];
  auto end = &sl->part[sl->n - 1];

  new (dst) Particle(std::move(*src));
  if (src != end) {
    new (src) Particle(std::move(*end));
  }

  sl->resize(sl->n - 1);
  return dst;
}

Particle *move_indexed_particle(ParticleList *dl, ParticleList *sl, int i) {
  assert(sl->n > 0);
  assert(i < sl->n);
  int re = dl->resize(dl->n + 1);
  Particle *dst = &dl->part[dl->n - 1];
  Particle *src = &sl->part[i];
  Particle *end = &sl->part[sl->n - 1];

  new (dst) Particle(std::move(*src));

  if (re) {
    update_local_particles(dl);
  } else {
    set_local_particle_data(dst->p.identity, dst);
  }
  if (src != end) {
    new (src) Particle(std::move(*end));
  }

  if (sl->resize(sl->n - 1)) {
    update_local_particles(sl);
  } else if (src != end) {
    set_local_particle_data(src->p.identity, src);
  }
  return dst;
}

Particle extract_indexed_particle(ParticleList *sl, int i) {
  assert(sl->n > 0);
  assert(i < sl->n);
  Particle *src = &sl->part[i];
  Particle *end = &sl->part[sl->n - 1];

  Particle p = std::move(*src);

  set_local_particle_data(p.p.identity, nullptr);

  if (src != end) {
    new (src) Particle(std::move(*end));
  }

  if (sl->resize(sl->n - 1)) {
    update_local_particles(sl);
  } else if (src != end) {
    set_local_particle_data(src->p.identity, src);
  }
  return p;
}

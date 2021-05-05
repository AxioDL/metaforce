#include "VISIRenderer.hpp"
#include "VISIBuilder.hpp"
#include "athena/FileReader.hpp"
#include "logvisor/logvisor.hpp"

#include <png.h>

static logvisor::Module Log("visigen");

/* structure to store PNG image bytes */
struct mem_encode
{
  char *buffer;
  size_t size;
};

static void
my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  /* with libpng15 next line causes pointer deference error; use libpng12 */
  struct mem_encode* p=(struct mem_encode*)png_get_io_ptr(png_ptr); /* was png_ptr->io_ptr */
  size_t nsize = p->size + length;

  /* allocate or grow buffer */
  if(p->buffer)
    p->buffer = (char *) realloc(p->buffer, nsize);
  else
    p->buffer = (char *) malloc(nsize);

  if(!p->buffer)
    png_error(png_ptr, "Write Error");

  /* copy new bytes to end of buffer */
  memcpy(p->buffer + p->size, data, length);
  p->size += length;
}

/*
   write an rgba image to a memory buffer in PNG format, without any fanciness.

   Params: rgba - the rgba values
       width - image width
       height - image height
       outsize - return for size of output buffer
   Returns: pointer to allocated buffer holding png data
*/
void *VISIRenderer::makePNGBuffer(unsigned char *rgba, int width, int height, size_t *outsize)
{
  int code = 0;
  // FILE *fp;
  png_structp png_ptr = 0;
  png_infop info_ptr =0;
  png_bytep row = 0;

  struct mem_encode state = {0, 0};

  *outsize = 0;

  // Initialize write structure
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fprintf(stderr, "Could not allocate write struct\n");
    code = 1;
    goto finalise;
  }

// Initialize info structure
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fprintf(stderr, "Could not allocate info struct\n");
    code = 1;
    goto finalise;
  }

  // Setup Exception handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "Error during png creation\n");
    code = 1;
    goto finalise;
  }

  // png_init_io(png_ptr, fp);

  /* if my_png_flush() is not needed, change the arg to NULL */
  png_set_write_fn(png_ptr, &state, my_png_write_data, NULL);

  // Write header (8 bit colour depth)
  png_set_IHDR(png_ptr, info_ptr, width, height,
               8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  // Set title
  /*
   if (title != NULL) {
   png_text title_text;
   title_text.compression = PNG_TEXT_COMPRESSION_NONE;
   title_text.key = "Title";
   title_text.text = title;
   png_set_text(png_ptr, info_ptr, &title_text, 1);
   }
   */

  png_write_info(png_ptr, info_ptr);

  // Allocate memory for one row (4 bytes per pixel - RGBA)
  row = (png_bytep) malloc(4 * width * sizeof(png_byte));

  // Write image data
  int x, y;
  for (y=0 ; y<height ; y++) {
    for (x=0 ; x<width ; x++) {
      //  setRGB(&(row[x*3]), buffer[y*width + x]);
      row[x*4] = rgba[(y*width +x)*4];
      row[x*4+1] = rgba[(y*width +x)*4+1];
      row[x*4+2] = rgba[(y*width +x)*4+2];
      row[x*4+3] = rgba[(y*width +x)*4+3];
//      row[x*4] = 255;
//      row[x*4+1] = 0;
//      row[x*4+2] = 255;
//      row[x*4+3] = 255;
    }
    png_write_row(png_ptr, row);
  }

  // End write
  png_write_end(png_ptr, NULL);

  finalise:
  // if (fp != NULL) fclose(fp);
  if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  if (row != NULL) free(row);

  *outsize = state.size;
  return state.buffer;

}

zeus::CColor VISIRenderer::ColorForIndex(uint32_t i) {
  i += 1;
  return zeus::CColor((i & 0xff) / 255.f, ((i >> 8) & 0xff) / 255.f, ((i >> 16) & 0xff) / 255.f, 1.f);
}

std::vector<VISIRenderer::Model::Vert> VISIRenderer::AABBToVerts(const zeus::CAABox& aabb, const zeus::CColor& color) {
  std::vector<Model::Vert> verts;
  verts.resize(8);

  for (int i = 0; i < 8; ++i)
    verts[i].color = color;

  verts[0].pos = aabb.min;
  verts[1].pos = {aabb.max.x(), aabb.min.y(), aabb.min.z()};
  verts[2].pos = {aabb.min.x(), aabb.min.y(), aabb.max.z()};
  verts[3].pos = {aabb.max.x(), aabb.min.y(), aabb.max.z()};
  verts[4].pos = {aabb.min.x(), aabb.max.y(), aabb.max.z()};
  verts[5].pos = aabb.max;
  verts[6].pos = {aabb.min.x(), aabb.max.y(), aabb.min.z()};
  verts[7].pos = {aabb.max.x(), aabb.max.y(), aabb.min.z()};

  return verts;
}

void VISIRenderer::Run(FPercent updatePercent) {
  m_updatePercent = updatePercent;

  if (!SetupShaders()) {
    m_return = 1;
    return;
  }

  if (m_argc < 3) {
    Log.report(logvisor::Error, FMT_STRING("Missing input/output file args"));
    m_return = 1;
    return;
  }

  ProcessType parentPid = 0;
  if (m_argc > 4)
#ifdef _WIN32
    parentPid = ProcessType(wcstoull(m_argv[4], nullptr, 10));
#else
    parentPid = ProcessType(strtoull(m_argv[4], nullptr, 10));
#endif

  uint32_t layer2LightCount = 0;
  {
    athena::io::FileReader r(m_argv[1]);
    if (r.hasError())
      return;

    uint32_t modelCount = r.readUint32Big();
    m_models.resize(modelCount);
    for (uint32_t i = 0; i < modelCount; ++i) {
      zeus::CColor color = ColorForIndex(i);
      Model& model = m_models[i];
      uint32_t topology = r.readUint32Big();
      model.topology = static_cast<hecl::HMDLTopology>(topology);

      uint32_t vertCount = r.readUint32Big();
      model.verts.reserve(vertCount);
      for (uint32_t j = 0; j < vertCount; ++j) {
        model.verts.emplace_back();
        Model::Vert& vert = model.verts.back();
        vert.pos = r.readVec3fBig();
        vert.color = color;
        m_totalAABB.accumulateBounds(vert.pos);
        model.aabb.accumulateBounds(vert.pos);
      }

      uint32_t surfCount = r.readUint32Big();
      model.surfaces.resize(surfCount);
      uint32_t curIdx = 0;
      for (uint32_t j = 0; j < surfCount; ++j) {
        Model::Surface& surf = model.surfaces[j];
        surf.first = curIdx;
        surf.count = r.readUint32Big();
        curIdx += surf.count;

        for (uint32_t k = 0; k < surf.count; ++k) {
          uint32_t idx = r.readUint32Big();
          model.idxs.push_back(idx);
        }

        surf.transparent = r.readBool();
      }
    }

    uint32_t entityCount = r.readUint32Big();
    m_entities.resize(entityCount);
    for (uint32_t i = 0; i < entityCount; ++i) {
      Entity& ent = m_entities[i];
      ent.entityId = r.readUint32Big();
      ent.aabb.min = r.readVec3fBig();
      ent.aabb.max = r.readVec3fBig();
    }

    uint32_t lightCount = r.readUint32Big();
    layer2LightCount = r.readUint32Big();
    m_lights.resize(lightCount);
    for (uint32_t i = 0; i < lightCount; ++i) {
      Light& light = m_lights[i];
      light.point = r.readVec3fBig();
    }
  }

  if (!SetupVertexBuffersAndFormats()) {
    m_return = 1;
    return;
  }

  VISIBuilder builder(*this);
  std::vector<uint8_t> dataOut =
      builder.build(m_totalAABB, m_models.size(), m_entities, m_lights, layer2LightCount, m_updatePercent, parentPid);
  if (dataOut.empty()) {
    m_return = 1;
    return;
  }

  athena::io::FileWriter w(m_argv[2]);
  w.writeUBytes(dataOut.data(), dataOut.size());
}

void VISIRenderer::Terminate() { m_terminate = true; }

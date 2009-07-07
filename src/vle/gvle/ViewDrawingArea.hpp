/**
 * @file vle/gvle/ViewDrawingArea.hpp
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2008 The VLE Development Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef VLE_GVLE_VIEWDRAWINGAREA_HPP
#define VLE_GVLE_VIEWDRAWINGAREA_HPP

#include <gtkmm/drawingarea.h>

namespace vle { namespace graph {

    class Model;
    class CoupledModel;

}} // namespace vle graph

namespace vle { namespace gvle {

    class GVLE;
    class View;
    class Modeling;

    class Color
    {
    public:
	double m_r; double m_g; double m_b;
	Color(double r = 0, double g = 0, double b = 0) :
	    m_r(r), m_g(g), m_b(b)
	    {}
    };

    /**
     * @brief A Gtk::DrawingArea to draw the gui::GModel onto the screen. This
     * objet is attached to a gui::View that manage menu and global Gtk::Window
     * around this Gtk::DrawingArea.
     */
    class ViewDrawingArea : public Gtk::DrawingArea
    {
    public:
        static const guint MODEL_WIDTH;
        static const guint MODEL_DECAL;
        static const guint MODEL_PORT;
        static const guint MODEL_PORT_SPACING_LABEL;
        static const guint MODEL_SPACING_PORT;
        static const guint PORT_SPACING_LABEL;
        static const guint MODEL_HEIGHT;
        static const guint CURRENT_MODEL_PORT;
        static const guint CURRENT_MODEL_SPACE;
        static const double ZOOM_FACTOR_SUP;
        static const double ZOOM_FACTOR_INF;
        static const double ZOOM_MIN;
        static const double ZOOM_MAX;
        static const guint SPACING_MODEL;
        static const guint SPACING_LINE;
        static const guint SPACING_MODEL_PORT;

        ViewDrawingArea(View* view);

        virtual ~ViewDrawingArea() {}

        void draw();

        //
        // MANAGE ZOOM / UNZOOM IN VIEW
        //

        /**
         * when zoom click, if left button then zoom, if right button then
         * unzoom, otherwise default size
         *
         * @param button 1 for left, 2 for right, otherwise other
         */
        void onZoom(int button);

        /**
         * Get current zoom value
         *
         * @return zoom value
         */
        inline double getZoom() const { return mZoom; }

        /** Add a constant to zoom limit by 4.0 */
        void addCoefZoom();

        /** Del a constant to zoom limit by 0.1 */
        void delCoefZoom();

        /** restore zoom to default value 1.0 */
        void restoreZoom();

	/*
	 * set a new coefficient to the zoom
	 * @parm coef the new coefficient
	 *
	 */
	void setCoefZoom(double coef);


        /**
         * Calculate a new Zoom factor using 2 points and apply to the
         * DrawingArea.
         *
         * @param xmin left position
         * @param ymin top position
         * @param xmax right position
         * @param ymax bottom position
         */
        void selectZoom(int xmin, int ymin, int xmax, int ymax);

        /** Calculate the new size of DrawingArea. */
        void newSize();

        void getCurrentModelInPosition(const std::string& p, int& x, int& y);
        void getCurrentModelOutPosition(const std::string& p, int& x, int& y);
        void getModelInPosition(graph::Model* model, const std::string& p,
                                int& x, int& y);
        void getModelOutPosition(graph::Model* model, const std::string& p,
                                 int& x, int& y);
        void calcSize(graph::Model* m);
        void calcRectSize();

	/**
	 * Export the view in image
	 */
	void exportPng(const std::string& filename);
	void exportPdf(const std::string& filename);
	void exportSvg(const std::string& filename);

	/**
	 * Order the models
	 */
	void onRandomOrder();

    private:

	typedef std::pair < int, int > Point;
	typedef std::vector < Point > StraightLine;

        class Connection
        {
        public:
            int xs, ys, xd, yd, x1, x2, y1, y3, y4;
            bool top;

            Connection(int xs, int ys, int xd, int yd,
                       int x1, int x2, int y1, int y3, int y4) :
                xs(xs), ys(ys), xd(xd), yd(yd), x1(x1), x2(x2),
                y1(y1), y3(y3), y4(y4), top(false)
            {}
        };

	void drawCurrentCoupledModel();
        void drawCurrentModelPorts();

        void preComputeConnection(int xs, int ys, int xd, int yd,
                                  int ytms, int ybms);
        void preComputeConnection(graph::Model* src,
                                  const std::string& srcport,
                                  graph::Model* dst,
                                  const std::string& dstport);
        void preComputeConnection();

        StraightLine computeConnection(int xs, int ys, int xd, int yd,
                                       int index);
        void computeConnection(graph::Model* src, const std::string& srcport,
                               graph::Model* dst, const std::string& dstport,
                               int index);
        void computeConnection();

        void highlightLine(int mx, int my);

        void drawConnection();
        void drawHighlightConnection();
        void drawChildrenModels();
	void drawChildrenModel(graph::Model* model,
			       Color color);
	void drawChildrenPorts(graph::Model* model,
			       Color color);
        void drawLines();
        void drawLink();
        void drawZoomFrame();

        bool on_button_press_event(GdkEventButton* event);
        bool on_button_release_event(GdkEventButton* event);
        bool on_configure_event(GdkEventConfigure* event);
        bool on_expose_event(GdkEventExpose* event);
        bool on_motion_notify_event(GdkEventMotion* event);
        void on_realize();

        void on_gvlepointer_button_1(graph::Model* mdl, bool state);
        void delUnderMouse(int x, int y);

        /**
         * add a link between two model prec Model and model under mouse x, y
         *
         * @param x mouse position
         * @param y mouse position
         */
        void addLinkOnButtonPress(int x, int y);
        void addLinkOnMotion(int x, int y);
        void addLinkOnButtonRelease(int x, int y);

	/**
	 * Change the color of the drawing
	 */
	void setColor(Color color);

        /**
         * Return nearest connection between all connection and mouse position
         */
        void delConnection();

        /**
         * @brief Add to the Gdk stack a call to expose event function with the
         * need to redraw the buffer with the draw() function.
         */
        void queueRedraw()
        { mNeedRedraw = true; queue_draw(); }

	GVLE*                           mGVLE;
        View*                           mView;
        graph::CoupledModel*            mCurrent;
        Modeling*                       mModeling;

        Gdk::Point                      mMouse;
        Gdk::Point                      mPrecMouse;

        int                             mHeight;
        int                             mWidth;
        int                             mRectHeight;
        int                             mRectWidth;
        double                          mZoom;
	double                          mOffset;
        Glib::RefPtr < Gdk::Pixmap >    mBuffer;
	Cairo::RefPtr<Cairo::Context>   mContext;
        Glib::RefPtr < Gdk::Window >    mWin;
        Glib::RefPtr < Gdk::GC >        mWingc;
        bool                            mIsRealized;

        /**
         * @brief True if, in the expose event, the buffer must be completely
         * redraw by calling the draw() function. False, just switch buffer.
         */
        bool                            mNeedRedraw;

        std::map < int, Point > mInPts;
        std::map < int, Point > mOutPts;
        std::vector < Connection > mConnections;
        std::map < Point, bool > mDirect;
        std::vector < StraightLine > mLines;
        std::vector < std::string > mText;
        int mHighlightLine;

	// Grid
	std::list < std::string > mGrid;
	int                       mCasesWidth;
	int                       mCasesHeight;
    };

}} // namespace vle gvle

#endif

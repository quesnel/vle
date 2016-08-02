/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2016 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2016 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2016 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and
 * contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <vle/vpz/SaxStackVpz.hpp>
#include <vle/vpz/SaxParser.hpp>
#include <vle/vpz/Structures.hpp>
#include <vle/vpz/Port.hpp>
#include <vle/vpz/Vpz.hpp>
#include <vle/vpz/CoupledModel.hpp>
#include <vle/vpz/AtomicModel.hpp>
#include <vle/utils/Exception.hpp>
#include <vle/utils/Tools.hpp>
#include <vle/utils/i18n.hpp>
#include <libxml/xpath.h>
#include <cstring>

namespace {

template <typename T>
inline void checkEmptyStack(const T& t)
{
    if (not t.empty())
        throw vle::utils::SaxParserError("Not empty vpz stack");
}

template <typename T>
inline void checkNotEmptyStack(const T& t)
{
    if (t.empty())
        throw vle::utils::SaxParserError("Empty vpz stack");
}

}

namespace vle { namespace vpz {

std::ostream& operator<<(std::ostream& out, const SaxStackVpz& stack)
{
    SaxStackVpz nv(stack);

    out << "SaxStackVpz:\n";
    while (not nv.m_stack.empty()) {
        out << "type " << nv.parent()->getType() << "\n";
        nv.pop();
    }

    out << "Xml:\n" << nv.m_vpz << "\n";
    return out;
}

void SaxStackVpz::clear()
{
    while (not m_stack.empty()) {
        if (parent()->isStructures() or
            parent()->isModel() or
            parent()->isIn() or
            parent()->isOut() or
            parent()->isState() or
            parent()->isInit() or
            parent()->isSubmodels() or
            parent()->isConnections() or
            parent()->isInternalConnection() or
            parent()->isInputConnection() or
            parent()->isOutputConnection() or
            parent()->isOrigin() or
            parent()->isDestination()) {
            delete parent();
        }
        pop();
    }
}

vpz::Vpz* SaxStackVpz::pushVpz(const xmlChar** att)
{
    ::checkEmptyStack(m_stack);

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"date") == 0) {
            m_vpz.project().setDate(xmlCharToString(att[i + 1]));
        } else if (xmlStrcmp(att[i], (const xmlChar*)"author") == 0) {
            m_vpz.project().setAuthor(xmlCharToString(att[i + 1]));
        } else if (xmlStrcmp(att[i], (const xmlChar*)"version") == 0) {
            m_vpz.project().setVersion(xmlCharToString(att[i + 1]));
        } else if (xmlStrcmp(att[i], (const xmlChar*)"instance") == 0) {
            m_vpz.project().setInstance(xmlCharToInt(att[i + 1]));
        }
    }

    push(&m_vpz);
    return &m_vpz;
}

void SaxStackVpz::pushStructure()
{
    ::checkNotEmptyStack(m_stack);
    checkParentIsVpz();

    push(new vpz::Structures());
}

void SaxStackVpz::pushModel(const xmlChar** att)
{
    ::checkNotEmptyStack(m_stack);
    checkParentOfModel();

    vpz::CoupledModel* cplparent = nullptr;

    if (parent()->isSubmodels()) {
        vpz::Base* sub = pop();
        vpz::Model* tmp = static_cast < vpz::Model* >(parent());
        cplparent = static_cast < vpz::CoupledModel* >(tmp->model());
        push(sub);
    }

    vpz::BaseModel* gmdl = nullptr;
    const xmlChar *conditions = nullptr, *dynamics = nullptr;
    const xmlChar *observables = nullptr, *width = nullptr, *height = nullptr;
    const xmlChar *x = nullptr, *y = nullptr;
    const xmlChar *type = nullptr, *name = nullptr;
    const xmlChar *debug = nullptr;

    for (int i = 0; att[i] != nullptr; i+=2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"type") == 0) {
            type = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"conditions") == 0) {
            conditions = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"dynamics") == 0) {
            dynamics = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"observables") == 0) {
            observables = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"x") == 0) {
            x = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"y") == 0) {
            y = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"width") == 0) {
            width = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"height") == 0) {
            height = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"debug") == 0) {
            debug = att[i + 1];
        }
    }

    if (name == nullptr) {
        throw utils::SaxParserError(_("Attribute name not defined"));
    }

    if (type == nullptr) {
        throw utils::SaxParserError(
            (fmt(_("Attribute type not defined for model '%1%'")) % name).str());
    }

    if (xmlStrcmp(type, (const xmlChar*)"atomic") == 0) {
        try {
            auto ptr = new vpz::AtomicModel(
                (const char*)name,
                cplparent,
                conditions ? xmlCharToString(conditions) : "",
                dynamics ? xmlCharToString(dynamics) : "",
                observables ? xmlCharToString(observables) : "");

            if (debug and (xmlStrcmp(debug, (const xmlChar*)"true") == 0))
                ptr->setDebug();

            gmdl = ptr;
        } catch(const utils::DevsGraphError& e) {
            throw utils::SaxParserError(
                      (fmt(_("Error build atomic model '%1%' with error: %2%"))
                       % name % e.what()).str());
        }
    } else if (xmlStrcmp(type, (const xmlChar*)"coupled") == 0) {
        try {
            gmdl = new vpz::CoupledModel((const char*)name, cplparent);
        } catch(const utils::DevsGraphError& e) {
            throw utils::SaxParserError(
                (fmt(_("Error build coupled model '%1%' with error: %2%"))
                 % name % e.what()).str());
        }
    } else {
        throw utils::SaxParserError(
            (fmt(_("Unknow model type %1%")) % (const char*)type).str());
    }

    auto  mdl = new vpz::Model();
    mdl->setModel(gmdl);
    buildModelGraphics(gmdl,
                       x ? xmlCharToString(x) : "",
                       y ? xmlCharToString(y) : "",
                       width ? xmlCharToString(width) : "",
                       height ? xmlCharToString(height) : "");

    if (parent()->isStructures()) {
        vpz().project().model().setModel(gmdl);
    } else if (parent()->isClass()) {
        reinterpret_cast < Class* >(parent())->setModel(gmdl);
    }

    push(mdl);
}

void SaxStackVpz::buildModelGraphics(vpz::BaseModel* mdl,
                                     const std::string& x,
                                     const std::string& y,
                                     const std::string& width,
                                     const std::string& height)
{
    if (not x.empty() and not y.empty()) {
        try {
            mdl->setX(std::stoi(x));
            mdl->setY(std::stoi(y));
        } catch (const std::exception& /* e */) {
            throw utils::SaxParserError(
                (fmt(_("Cannot convert x or y position for model %1%")) %
                 mdl->getName()).str());
        }
    }

    if (not width.empty() and not height.empty()) {
        try {
            mdl->setWidth(std::stoi(width));
            mdl->setHeight(std::stoi(height));
        } catch (const std::exception& /* e */) {
            throw utils::SaxParserError(
                (fmt(_("Cannot convert width or height for model %1%")) %
                 mdl->getName()).str());
        }
    }
}

void SaxStackVpz::pushPort(const xmlChar** att)
{
    if (m_stack.empty()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    if (parent()->isCondition()) {
        pushConditionPort(att);
    } else if (parent()->isObservable()) {
        pushObservablePort(att);
    } else {
        if (not (parent()->isIn() or parent()->isOut())) {
            throw utils::SaxParserError(_("Bad file format"));
        }

        vpz::Base* type = pop();

        if (not parent()->isModel()) {
            throw utils::SaxParserError(_("Bad file format"));
        }

        vpz::Model* mdl = static_cast < vpz::Model* >(parent());
        vpz::BaseModel* gmdl = mdl->model();
        std::string name;

        for (int i = 0; att[i] != nullptr; i += 2) {
            if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
                name = xmlCharToString(att[i + 1]);
            }
        }

        if (name.empty()) {
            throw utils::SaxParserError(_("Port without name"));
        }

        if (type->isIn()) {
            gmdl->addInputPort(name);
        } else if (type->isOut()) {
            gmdl->addOutputPort(name);
        }
        push(type);
    }
}

void SaxStackVpz::pushPortType(const char* att)
{
    if (m_stack.empty() or not parent()->isModel()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    vpz::Base* prt = nullptr;

    if (std::strcmp(att, "in") == 0) {
        prt = new vpz::In();
    } else if (std::strcmp(att, "out") == 0) {
        prt = new vpz::Out();
    } else if (std::strcmp(att, "state") == 0) {
        prt = new vpz::State();
    } else if (std::strcmp(att, "init") == 0) {
        prt = new vpz::Init();
    } else {
        throw utils::SaxParserError(
            (fmt(_("Unknow port type %1%.")) % att).str());
    }
    push(prt);
}

void SaxStackVpz::pushSubModels()
{
    if (m_stack.empty() or not parent()->isModel()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    auto  sub = new vpz::Submodels();
    push(sub);
}

void SaxStackVpz::pushConnections()
{
    if (m_stack.empty() or not parent()->isModel()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    auto  cnts = new vpz::Connections();
    push(cnts);
}

void SaxStackVpz::pushConnection(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isConnections()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* type = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"type") == 0) {
            type = att[i + 1];
        }
    }

    if (type == nullptr) {
        throw utils::SaxParserError(
            _("Connection tag does not have an attribue 'type'"));
    }

    vpz::Base* cnt = nullptr;
    if (xmlStrcmp(type, (const xmlChar*)"internal") == 0) {
        cnt = new vpz::InternalConnection();
    } else if (xmlStrcmp(type, (const xmlChar*)"input") == 0) {
        cnt = new vpz::InputConnection();
    } else if (xmlStrcmp(type, (const xmlChar*)"output") == 0) {
        cnt = new vpz::OutputConnection();
    } else {
        throw utils::SaxParserError(
            (fmt(_( "Unknow connection type %1%")) % type).str());
    }
    push(cnt);
}

void SaxStackVpz::pushOrigin(const xmlChar** att)
{
    if (m_stack.empty() or (not parent()->isInternalConnection()
        and not parent()->isInputConnection()
        and not parent()->isOutputConnection())) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* model = nullptr;
    const xmlChar* port = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"model") == 0) {
            model = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"port") == 0) {
            port = att[i + 1];
        }
    }

    if (not model or not port) {
        throw utils::SaxParserError(
            _("Origin tag does not have attributes 'model' or 'port'"));
    }

    vpz::Base* orig = new vpz::Origin((const char*)model, (const char*)port);
    push(orig);
}

void SaxStackVpz::pushDestination(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isOrigin()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* model = nullptr;
    const xmlChar* port = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"model") == 0) {
            model = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"port") == 0) {
            port = att[i + 1];
        }
    }

    if (not model or not port) {
        throw utils::SaxParserError(
            _("Destination tag does not have attributes 'model' or 'port'"));
    }

    vpz::Base* dest = new vpz::Destination((const char*)model, (const char*)port);
    push(dest);
}

void SaxStackVpz::buildConnection()
{
    if (m_stack.empty()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    if (not parent()->isDestination()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    vpz::Destination* dest = static_cast < vpz::Destination* >(pop());

    if (not parent()->isOrigin()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    vpz::Origin* orig = static_cast < vpz::Origin* >(pop());

    if (not (parent()->isInternalConnection() or
             parent()->isInputConnection() or
             parent()->isOutputConnection())) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    vpz::Base* cnt = pop();

    if (not parent()->isConnections()) {
        throw utils::SaxParserError(_("Bad file format"));
    }
    vpz::Base* cntx = pop();

    if (not parent()->isModel()) {
        throw utils::SaxParserError(_("Bad file format"));
    }
    vpz::Model* model = static_cast < vpz::Model* >(parent());

    vpz::CoupledModel* cpl = static_cast < vpz::CoupledModel*
        >(model->model());

    if (cnt->isInternalConnection()) {
        cpl->addInternalConnection(orig->model, orig->port, dest->model,
                                   dest->port);
    } else if (cnt->isInputConnection()) {
        cpl->addInputConnection(orig->port, dest->model, dest->port);
    } else if (cnt->isOutputConnection()) {
        cpl->addOutputConnection(orig->model, orig->port, dest->port);
    }

    push(cntx);
    delete dest;
    delete orig;
    delete cnt;
}

void SaxStackVpz::pushDynamics()
{
    if (m_stack.empty() or not parent()->isVpz()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    push(&m_vpz.project().dynamics());
}

void SaxStackVpz::pushDynamic(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isDynamics()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* name = nullptr;
    const xmlChar* package = nullptr;
    const xmlChar* library = nullptr;
    const xmlChar* language = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"package") == 0) {
            package = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"library") == 0) {
            library = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"language") == 0) {
            language = att[i + 1];
        }
    }

    if (not name or not library) {
        throw utils::SaxParserError(
            _("Dynamic tag does not have 'name' or 'library' attribute"));
    }

    vpz::Dynamic dyn(xmlCharToString(name));
    dyn.setLibrary(xmlCharToString(library));

    if (package) {
        dyn.setPackage(xmlCharToString(package));
    } else {
        dyn.setPackage("");
    }

    if (language) {
        dyn.setLanguage(xmlCharToString(language));
    } else {
        dyn.setLanguage("");
    }

    vpz::Dynamics* dyns(static_cast < Dynamics* >(parent()));
    dyns->add(dyn);
}

void SaxStackVpz::pushExperiment(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isVpz()) {
        throw utils::SaxParserError(_("Bad file format"));
    }



    vpz::Experiment& exp(m_vpz.project().experiment());
    push(&exp);

    const xmlChar* name = nullptr;
    const xmlChar* combination = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"combination") == 0) {
            combination = att[i + 1];
        }
    }

    exp.setName(xmlCharToString(name));

    if (combination) {
        exp.setCombination(xmlCharToString(combination));
    }
}

void SaxStackVpz::pushConditions()
{
    if (m_stack.empty() or not parent()->isExperiment()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    push(&m_vpz.project().experiment().conditions());
}

void SaxStackVpz::pushCondition(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isConditions()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    vpz::Conditions& cnds(m_vpz.project().experiment().conditions());

    const xmlChar* name = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        }
    }

    if (not name) {
        throw utils::SaxParserError(
            _("Condition tag does not have attribute 'name'"));
    }

    vpz::Condition newcondition(xmlCharToString(name));
    vpz::Condition& cnd(cnds.add(newcondition));
    push(&cnd);
}

void SaxStackVpz::pushConditionPort(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isCondition()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* name = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        }
    }

    if (not name) {
        throw utils::SaxParserError(
            _("Condition port tag does not have attribute 'name'"));
    }

    vpz::Condition* cnd(static_cast < vpz::Condition* >(parent()));
    cnd->add(xmlCharToString(name));
}

value::Set& SaxStackVpz::popConditionPort()
{
    if (m_stack.empty() or not parent()->isCondition()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    vpz::Condition* cnd(static_cast < vpz::Condition* >(parent()));
    value::Set& vals(cnd->lastAddedPort());

    return vals;
}

void SaxStackVpz::pushViews()
{
    if (m_stack.empty() or not parent()->isExperiment()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    push(&m_vpz.project().experiment().views());
}

void SaxStackVpz::pushOutputs()
{
    if (m_stack.empty() or not parent()->isViews()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    push(&m_vpz.project().experiment().views().outputs());
}

void SaxStackVpz::popOutput()
{
    if (m_stack.empty() or not parent()->isOutput()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    pop();
}

void SaxStackVpz::pushOutput(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isOutputs()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* name = nullptr;
    const xmlChar* format = nullptr;
    const xmlChar* plugin = nullptr;
    const xmlChar* location = nullptr;
    const xmlChar* package = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"format") == 0) {
            format = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"plugin") == 0) {
            plugin = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"location") == 0) {
            location = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"package") == 0) {
            package = att[i + 1];
        }
    }

    Outputs& outs(m_vpz.project().experiment().views().outputs());

    if (xmlStrcmp(format, (const xmlChar*)"local") == 0) {
        Output& result = outs.addLocalStream(
            xmlCharToString(name),
            location ? xmlCharToString(location) : std::string(),
            xmlCharToString(plugin),
            package ? xmlCharToString(package) : std::string());
        push(&result);
    } else if (xmlStrcmp(format, (const xmlChar*)"distant") == 0) {
        Output& result = outs.addDistantStream(
            xmlCharToString(name),
            location ? xmlCharToString(location) : std::string(),
            xmlCharToString(plugin),
            package ? xmlCharToString(package) : std::string());
        push(&result);
    } else {
        throw utils::SaxParserError(
            (fmt(_("Output tag does not define a '%1%' format")) % name).str());
    }
}

void SaxStackVpz::pushView(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isViews()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* name = nullptr;
    const xmlChar* type = nullptr;
    const xmlChar* output = nullptr;
    const xmlChar* timestep = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"type") == 0) {
            type = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"output") == 0) {
            output = att[i + 1];
        } else if (xmlStrcmp(att[i], (const xmlChar*)"timestep") == 0) {
            timestep = att[i + 1];
        }
    }

    Views& views(m_vpz.project().experiment().views());

    if (xmlStrcmp(type, (const xmlChar*)"timed") == 0) {
        if (not timestep) {
            throw utils::SaxParserError(
                _("View tag does not have a timestep attribute"));
        }
        views.addTimedView(xmlCharToString(name),
                           xmlXPathCastStringToNumber(timestep),
                           xmlCharToString(output));
    } else {
        using ustring = std::basic_string<unsigned char>;

        ustring typestr(type);
        ustring::size_type begin { 0 };
        ustring::size_type it = typestr.find(',');
        View::Type viewtype = View::NOTHING;

        while (begin != std::string::npos) {
            auto tmp = typestr.substr(begin, it - 1);
            if (tmp == (const xmlChar*)"event")
                viewtype = View::INTERNAL | View::CONFLUENT | View::EXTERNAL;
            else if (tmp == (const xmlChar*)"output")
                viewtype |= View::OUTPUT;
            else if (tmp == (const xmlChar*)"internal")
                viewtype |= View::INTERNAL;
            else if (tmp == (const xmlChar*)"external")
                viewtype |= View::EXTERNAL;
            else if (tmp == (const xmlChar*)"confluent")
                viewtype |= View::CONFLUENT;
            else if (tmp == (const xmlChar*)"finish")
                viewtype |= View::FINISH;
            else
                throw utils::SaxParserError(
                    (fmt(_("View tag does not accept type '%1%'"))
                     % type).str());

            if (it != std::string::npos)
                begin = std::min(it + 1, typestr.size());
            else
                begin = std::string::npos;
        }

        views.addEventView(xmlCharToString(name),
                           static_cast<View::Type>(viewtype),
                           xmlCharToString(output));
   }
}

void SaxStackVpz::pushAttachedView(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isObservablePort()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    pushObservablePortOnView(att);
}

void SaxStackVpz::pushObservables()
{
    if (m_stack.empty() or not parent()->isViews()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    Observables& obs(m_vpz.project().experiment().views().observables());
    push(&obs);
}

void SaxStackVpz::pushObservable(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isObservables()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* name = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        }
    }

    if (not name) {
        throw utils::SaxParserError(
            _("Observable tag does not have attribute 'name'"));
    }

    Views& views(m_vpz.project().experiment().views());
    Observable& obs(views.addObservable(xmlCharToString(name)));
    push(&obs);
}

void SaxStackVpz::pushObservablePort(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isObservable()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* name = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        }
    }

    if (not name) {
        throw utils::SaxParserError(
            _("Observable port tag does not have attribute 'name'"));
    }

    vpz::Observable* out(static_cast < vpz::Observable* >(parent()));
    vpz::ObservablePort& ports(out->add(xmlCharToString(name)));
    push(&ports);
}

void SaxStackVpz::pushObservablePortOnView(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isObservablePort()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* name = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        }
    }

    if (not name) {
        throw utils::SaxParserError(
            _("Observable port view tag does not have attribute 'name'"));
    }

    vpz::ObservablePort* port(static_cast < vpz::ObservablePort* >(parent()));
    port->add(xmlCharToString(name));
}

void SaxStackVpz::pushClasses()
{
    if (m_stack.empty() or not parent()->isVpz()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    push(&m_vpz.project().classes());
}

void SaxStackVpz::pushClass(const xmlChar** att)
{
    if (m_stack.empty() or not parent()->isClasses()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    const xmlChar* name = nullptr;

    for (int i = 0; att[i] != nullptr; i += 2) {
        if (xmlStrcmp(att[i], (const xmlChar*)"name") == 0) {
            name = att[i + 1];
        }
    }

    if (not name) {
        throw utils::SaxParserError(
            _("Class tag does not have attribute 'name'"));
    }

    Class& cls = m_vpz.project().classes().add(xmlCharToString(name));
    push(&cls);
}

void SaxStackVpz::popClasses()
{
    if (m_stack.empty() or not parent()->isClasses()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    pop();
}

void SaxStackVpz::popClass()
{
    if (m_stack.empty() or not parent()->isClass()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    pop();
}

vpz::Base* SaxStackVpz::pop()
{
    vpz::Base* top = parent();
    m_stack.pop_front();
    return top;
}

const vpz::Base* SaxStackVpz::top() const
{
    if (m_stack.empty()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    return parent();
}

vpz::Base* SaxStackVpz::top()
{
    if (m_stack.empty()) {
        throw utils::SaxParserError(_("Bad file format"));
    }

    return parent();
}

Class* SaxStackVpz::getLastClass() const
{
    std::list < vpz::Base* >::const_iterator it;

    it = std::find_if(m_stack.begin(), m_stack.end(), Base::IsClass());
    return (it != m_stack.end()) ? reinterpret_cast < Class* >(*it) : nullptr;
}

void SaxStackVpz::checkParentIsVpz() const
{
    if (not parent()->isVpz()) {
        throw utils::SaxParserError(
            _("Parent is not Vpz element"));
    }
}

void SaxStackVpz::checkParentIsClass() const
{
    if (not parent()->isClass()) {
        throw utils::SaxParserError(
            _("Parent is not Class element"));
    }
}

void SaxStackVpz::checkParentIsSubmodels() const
{
    if (not parent()->isSubmodels()) {
        throw utils::SaxParserError(
            _("Parent is not Submodels element"));
    }
}

void SaxStackVpz::checkParentIsStructures() const
{
    if (not parent()->isStructures()) {
        throw utils::SaxParserError(
            _("Parent is not Structures element"));
    }
}

void SaxStackVpz::checkParentOfModel() const
{
    if (not parent()->isClass() and not parent()->isSubmodels() and not
        parent()->isStructures()) {
        throw utils::SaxParserError(
            _("Parent of model is not class, submodel or structure"));
    }
}

}} // namespace vle vpz

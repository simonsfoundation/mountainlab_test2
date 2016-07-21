#include "actionfactory.h"

void ActionFactory::addToToolbar(ActionType action, QWidget* container, QObject* receiver, const char* signalOrSlot)
{
    Q_ASSERT(container);
    Q_ASSERT(receiver);
    Q_ASSERT(signalOrSlot);

    QString name;
    QString tooltip;
    QIcon icon;

    switch (action) {
    case ActionType::ZoomIn:
        name = "Zoom In";
        tooltip = "Zoom in. Alternatively, use the mouse wheel.";
        icon = QIcon(":/images/zoom-in.png");
        break;
    case ActionType::ZoomOut:
        name = "Zoom Out";
        tooltip = "Zoom out. Alternatively, use the mouse wheel.";
        icon = QIcon(":/images/zoom-out.png");
        break;
    case ActionType::ZoomInVertical:
        name = "Vertical Zoom In";
        tooltip = "Vertical zoom in. Alternatively, use the UP arrow.";
        icon = QIcon(":/images/vertical-zoom-in.png");
        break;
    case ActionType::ZoomOutVertical:
        name = "Vertical Zoom Out";
        tooltip = "Vertical zoom out. Alternatively, use the DOWN arrow.";
        icon = QIcon(":/images/vertical-zoom-out.png");
        break;
    case ActionType::PanLeft:
        name = "<-Pan";
        tooltip = "Pan Left";
        icon = QIcon();
        break;
    case ActionType::PanRight:
        name = "Pan->";
        tooltip = "Pan Right";
        icon = QIcon();
        break;
    default:
        break;
    }

    QAction* a = new QAction(icon, name, container);
    a->setProperty("action_type", "toolbar");
    a->setToolTip(tooltip);
    container->addAction(a);
    QObject::connect(a, SIGNAL(triggered(bool)), receiver, signalOrSlot);
}

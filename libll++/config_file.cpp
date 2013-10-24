#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "memory.h"
#include "config_file.h"
#include "file_io.h"
#include "guard.h"

#define BUFF_SIZE   4096
namespace ll {

struct config_file {
    list_entry _entry;
    file_io _fd;
    config_pos _pos;
    char *_endp;
    char *_p;
    page *_chunk;

public:
    config_file() : _fd() {}

    void load() {
        int n = ::read(_fd, _chunk->firstp, _chunk->space());
        if (n <= 0) {
            _endp = nullptr;
        }
        else {
            _p = _chunk->firstp;
            _endp = _p + n;
        }
    }

    int open(const char *filename) {
        ll_sys_failed_return(_fd = ::open(filename, O_RDONLY));

        _chunk = page_allocator::global()->alloc();
        
        load();

        _pos.file = filename;
        _pos.line = 1;
        _pos.column = 0;

        return ok;
    }

    void close() {
        _fd.close();
        page_allocator::global()->free(_chunk);
    }

    int getc() {
        while (1) {
            if (!_endp) {
                return EOF;
            }
            if (_p < _endp) {
                int c = *_p++; 
                if (c == '\r') {
                    continue;
                }
                if (c == '\n') {
                    _pos.line++;
                    _pos.column = 0;
                }
                else {
                    _pos.column++;
                }
                return c;
            }

            load();
        }
    }
};

struct config_loader {
    enum {
        token_string = 256,
        token_block_end,
        token_include,
    };
    struct node {
        clist_entry entry;
        void *data;
    };

    obstack *_pool;
    config_text *_t;
    int _c;

    ll_list(node, entry) _list;
    ll_list(node, entry) _recycle;
    ll_list(config_file, _entry) _files;
    config_doc *_doc;
    config_item *_cur;
public:
    config_loader(obstack *pool, config_doc *doc) : 
        _pool(pool), _list(), _recycle(), _files(), _doc(doc), _cur()
    {
    }

    ~config_loader() {
    }

    void list_push(void *data) {
        node *p = _recycle.pop_front();
        if (!p) {
            p = (node*)_pool->alloc(sizeof(node));
        }
        p->data = data;
        _list.push_back(p);
    }

    void *list_pop() {
        node *p = _list.pop_front();
        if (!p) {
            return nullptr;
        }
        void *data = p->data;
        _recycle.push_front(p);
        return data;
    }

    int open_file(const char *filename) {
        for (auto& f : _files) {
            if (strcmp(f._pos.file, filename) == 0) {
                cout << "recursive include file." << endl;
                return fail;
            }
        }
        config_file *f = _new<config_file>(_pool);
        if (ll_failed(f->open(filename))) {
            cout << "open file failed." << endl;
            return fail;
        }

        _files.push_front(f);
        return ok;
    }

    void getc() {
        _c = _files.front()->getc();
    }

    config_pos &getpos() {
        return _files.front()->_pos;
    }

    int get_token() {
        while (1) {
            switch (_c) {
            case ' ':
            case '\t':
                getc();
                break;
            case '#':
                do {
                    getc();
                } while (_c != '\n' && _c != EOF);
                break;
            case EOF:
            case '>':
            case '\n': {
                int c = _c;
                getc();
                return c;
            }
            case '<': {
                getc();
                switch (_c) {
                case '/':
                    getc();
                    return token_block_end;
                case '!':
                    getc();
                    return token_include;
                default:
                    return '<';
                }
            }
            case '\"': 
                _t = _new<config_text>(_pool);
                _t->loc.start = getpos();
                getc();

                while (1) {
                    switch (_c) {
                    case EOF:
                    case '\n':
                        cout << "expected '\"'." << endl;
                        return fail;
                    case '\\':
                        getc();
                        if (_c != '\\' && _c != '"') {
                            cout << "expected \\\\ or \\\"." << endl;
                            return fail;
                        }
                        _pool->grow((char)_c);
                        getc();
                        break;
                    case '"':
                        _pool->grow('\0');
                        _t->_text = (const char*)_pool->finish();
                        _t->loc.end = getpos();
                        getc();
                        return token_string;
                    default:
                        _pool->grow((char)_c);
                        getc();
                    }
                }
            default:
                _t = _new<config_text>(_pool);
                _t->loc.start = getpos();
                while (1) {
                    switch (_c) {
                    case EOF:
                    case '#':
                    case '\n':
                    case '\t':
                    case ' ':
                    case '"':
                    case '<':
                    case '>':
                        _pool->grow('\0');
                        _t->_text = (const char*)_pool->finish();
                        return token_string;
                    default:
                        _pool->grow((char)_c);
                        _t->loc.end = getpos();
                        getc();
                    }
                }
                break;
            }
        }
    }

    void push_item(config_item *item) {
        if (_cur) {
            _cur->children.push_back(item);
        }
        else {
            _doc->items.push_back(item);
        }
    }

    int read_item() {
        int tk;
        config_item *item;
        ll_failed_return(tk = get_token());

        switch (tk) {
        case EOF:
        case '\n':
            return ok;
        case token_string: 
        {
            item = _new<config_item>(_pool, _cur);
            item->name = _t;
            while (1) {
                ll_failed_return(tk = get_token());
                switch (tk) {
                case EOF:
                case '\n':
                    if (item->value_count) {
                        config_text *t;
                        while ((t = (config_text*)list_pop())) {
                            _pool->grow(t);
                        }
                        item->values = (config_text**)_pool->finish();
                    }
                    else {
                        item->values = nullptr;
                    }
                    push_item(item);
                    return ok;
                case token_string:
                    item->value_count++;
                    list_push(_t);
                    break;
                default:
                    cout << "expected string or new line or EOF." << endl;
                    return fail;
                }
            }
            break;
        }
        case '<': 
        {
            ll_failed_return(tk = get_token());
            if (tk != token_string) {
                cout << "expected string." << endl;
                return fail;
            }
            item = _new<config_item>(_pool, _cur);
            item->name = _t;
            while (1) {
                ll_failed_return(tk = get_token());
                switch (tk) {
                case '>':
                    ll_failed_return(tk = get_token());
                    if (tk != '\n' && tk != EOF) {
                        cout << "expected new line or EOF." << endl;
                        return fail;
                    }
                    if (item->value_count) {
                        config_text **p, *t;
                        p = item->values = (config_text**)_pool->alloc(sizeof(config_text*) * item->value_count);
                        while ((t = (config_text*)list_pop())) {
                            *p++ = t;
                        }
                    }
                    else {
                        item->values = nullptr;
                    }
                    push_item(item);
                    _cur = item;
                    return ok;
                case token_string:
                    item->value_count++;
                    list_push(_t);
                    break;
                default:
                    cout << "expected string or '>'." << endl;
                    return fail;
                }
            }
            break;
        }
        case token_block_end: 
        {
            ll_failed_return(tk = get_token());
            if (tk != token_string) {
                cout << "expected string." << endl;
                return fail;
            }
            ll_failed_return(tk = get_token());
            if (tk != '>') {
                cout << "expected '>'." << endl;
                return fail;
            }
            ll_failed_return(tk = get_token());
            if (tk != '\n' && tk != EOF) {
                cout << "expected new line or EOF." << endl;
                return fail;
            }
            if (_cur == nullptr || 
                strcmp(*static_cast<config_item*>(_cur)->name, *_t) != 0) {
                cout << "unmatching item block." << endl;
                return fail;
            }
            cout << "oooooofffffffffffffff " << _cur->children.empty() << endl;
            _cur = _cur->_parent;
            return ok;
        }
        case token_include: 
        {
            ll_failed_return(tk = get_token());
            if (tk != token_string) {
                cout << "expected string." << endl;
                return fail;
            }
            ll_failed_return(tk = get_token());
            if (tk != '>') {
                cout << "expected '>'." << endl;
                return fail;
            }
            ll_failed_return(tk = get_token());
            if (tk != '\n' && tk != EOF) {
                cout << "expected new line or EOF." << endl;
                return fail;
            }
            ll_failed_return(open_file(*_t));
            return ok;
        }
        }
        return 0;
    }
    
    int load() {
        getc();
        while (1) {
            if (_c == EOF) {
                config_file *f = _files.pop_front();
                f->close();
                if (_files.empty()) {
                    break;
                }
            }
            ll_failed_return(read_item());
        }
        return ok;
    }

    int load(const char *filename) {
        ll_failed_return(open_file(_pool->strdup(filename)));
        int n = load();
        config_file *f;
        while ((f = _files.pop_front())) {
            f->close();
        }
        return n;
    }
};

config_doc::config_doc() : _pool(), items()
{
}

config_doc::~config_doc()
{
    if (_pool) {
        _delete<obstack>(_pool);
    }
}

void config_doc::clear() 
{
    if (_pool) {
        items.init();
        _pool->clear();
    }
}

int config_doc::load(const char *filename) 
{
    if (!_pool) {
        _pool = _new<obstack>();
    } else {
        clear();
    }

    config_loader loader(_pool, this);
    return loader.load(filename);
}

}

